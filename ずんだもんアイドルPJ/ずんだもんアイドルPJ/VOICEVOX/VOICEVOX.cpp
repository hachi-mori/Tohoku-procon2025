// SynthesizeFromJSONFile.cpp
# include "../stdafx.h"

namespace VOICEVOX
{
	// 話者情報を取得する
	[[nodiscard]]
	Array<Speaker> GetSpeakers(const Duration timeout)
	{
		constexpr URLView url = U"http://localhost:50021/speakers";

		AsyncHTTPTask task = SimpleHTTP::GetAsync(url, {});

		Stopwatch stopwatch{ StartImmediately::Yes };

		while (not task.isReady())
		{
			if (timeout <= stopwatch)
			{
				task.cancel();
				return{};
			}

			System::Sleep(1ms);
		}

		if (not task.getResponse().isOK())
		{
			return{};
		}

		const JSON json = task.getAsJSON();
		Array<Speaker> speakers;

		for (auto&& [i, speaker] : json)
		{
			Speaker s;
			s.name = speaker[U"name"].get<String>();

			for (auto&& [k, style] : speaker[U"styles"])
			{
				Speaker::Style st;
				st.name = style[U"name"].get<String>();
				st.id = style[U"id"].get<int32>();
				s.styles.push_back(st);
			}

			speakers.push_back(s);
		}

		return speakers;
	}

	constexpr double kFrameRate = 93.75;

	String MidiToName(int midi)
	{
		static const String names[12] = {
			U"C", U"C#", U"D", U"D#", U"E", U"F",
			U"F#", U"G", U"G#", U"A", U"A#", U"B"
		};
		const int octave = midi / 12 - 1;
		return names[midi % 12] + Format(octave);
	}

	int CalcFrameLen(int64 ticks, double bpm, double tpqn, double& carry)
	{
		double beats = ticks / tpqn;
		double seconds = beats * (60.0 / bpm);
		double rawFrames = seconds * kFrameRate;
		double total = rawFrames + carry;
		int frameLen = static_cast<int>(std::floor(total + 0.5));
		carry = total - frameLen;
		return Max(1, frameLen);
	}

	// VOICEVOX プロジェクトファイル (VVProj) を Score JSON 形式に変換する
	[[nodiscard]]
    void ConvertVVProjToScoreJSON(const FilePath& vvprojPath, const FilePath& outJsonPath)
	{
		const JSON src = JSON::Load(vvprojPath);
		const JSON& song = src[U"song"];

		// 解像度(tpqn)
		double tpqn = 480.0;
		if (song[U"tpqn"].isNumber())
		{
			tpqn = song[U"tpqn"].get<double>();
		}

		// BPM の取得 (最初の要素)
		double bpm = 120.0;
		if (song[U"tempos"].isArray())
		{
			for (const auto& tempo : song[U"tempos"].arrayView())
			{
				bpm = tempo[U"bpm"].get<double>();
				break;
			}
		}

		Array<JSON> outNotes;
		double carry = 0.0;

		// 最初に 2-frame の休符
		{
			JSON rest;
			rest[U"frame_length"] = 2;
			rest[U"key"] = JSON();
			rest[U"lyric"] = U"";
			rest[U"notelen"] = U"R";
			outNotes << rest;
		}

		// トラック取得 (最初のトラックのみ)
		if (song[U"tracks"].isObject())
		{
			for (const auto& trackPair : song[U"tracks"])  // オブジェクトのイテレータ
			{
				const JSON& track = trackPair.value;
				if (track[U"notes"].isArray())
				{
					int64 prevEnd = 0;
					for (const auto& n : track[U"notes"].arrayView())
					{
						const int64 pos = n[U"position"].get<int64>();
						const int64 dur = n[U"duration"].get<int64>();
						const int    midi = n[U"noteNumber"].get<int>();
						const String lyric = n[U"lyric"].getString();

						// ギャップ(休符)
						if (const int64 gapTicks = pos - prevEnd; gapTicks > 0)
						{
							int gapFrames = CalcFrameLen(gapTicks, bpm, tpqn, carry);
							JSON gap;
							gap[U"frame_length"] = gapFrames;
							gap[U"key"] = JSON();
							gap[U"lyric"] = U"";
							gap[U"notelen"] = U"R";
							outNotes << gap;
						}

						// 実音符
						int noteFrames = CalcFrameLen(dur, bpm, tpqn, carry);
						JSON note;
						note[U"frame_length"] = noteFrames;
						note[U"key"] = midi;
						note[U"lyric"] = lyric;
						note[U"notelen"] = MidiToName(midi);
						outNotes << note;

						prevEnd = pos + dur;
					}
				}
				break;  // 最初のトラックのみ
			}
		}

		// 最後に 2-frame の休符
		{
			JSON rest;
			rest[U"frame_length"] = 2;
			rest[U"key"] = JSON();
			rest[U"lyric"] = U"";
			rest[U"notelen"] = U"R";
			outNotes << rest;
		}

		// 結果を保存
		JSON result;
		result[U"notes"] = outNotes;
		result.save(outJsonPath);
	}

	// JSON ファイルから音声合成を行う
	[[nodiscard]]
	bool SynthesizeFromJSONFile(const FilePath& jsonFilePath, const FilePath& savePath, const URL& synthesisURL, const Duration timeout)
	{
		// JSON ファイルを読み込む
		const JSON query = JSON::Load(jsonFilePath);

		if (not query)
		{
			Console(U"JSON ファイルの読み込みに失敗しました。");
			return false;
		}

		// JSON データを UTF-8 フォーマットに変換
		const std::string data = query.formatUTF8Minimum();
		const HashTable<String, String> headers{ { U"Content-Type", U"application/json" } };

		// 非同期 POST リクエストを送信
		AsyncHTTPTask task = SimpleHTTP::PostAsync(synthesisURL, headers, data.data(), data.size(), savePath);
		Stopwatch stopwatch{ StartImmediately::Yes };

		// リクエスト完了またはタイムアウトまで待機
		while (not task.isReady())
		{
			if (timeout <= stopwatch)
			{
				task.cancel();

				// タイムアウト時に不完全なファイルを削除
				if (FileSystem::IsFile(savePath))
				{
					FileSystem::Remove(savePath);
				}

				Console(U"リクエストがタイムアウトしました。");
				return false;
			}

			System::Sleep(1ms);
		}

		// レスポンスが成功したかを確認
		if (not task.getResponse().isOK())
		{
			// 失敗時に不完全なファイルを削除
			if (FileSystem::IsFile(savePath))
			{
				FileSystem::Remove(savePath);
			}

			Console(U"リクエストが失敗しました。");
			return false;
		}

		Console(U"ファイルが保存されました: " + savePath);
		return true;
	}

	// ScoreQuery→SingQuery
	// SingQuery→音声ファイル
	// 2段階を行うラッパー関数
	[[nodiscard]]
	bool SynthesizeFromJSONFileWrapper(const FilePath& inputPath, const FilePath& intermediatePath, const FilePath& outputPath, const URL& queryURL, const URL& synthesisURL)
	{
		if (!VOICEVOX::SynthesizeFromJSONFile(inputPath, intermediatePath, queryURL)) {
			Console(U"SingQuery の作成に失敗しました。");
			return false;
		}

		if (!VOICEVOX::SynthesizeFromJSONFile(intermediatePath, outputPath, synthesisURL)) {
			Console(U"音声合成に失敗しました。");
			return false;
		}

		Console(U"音声合成が成功しました。");
		return true;
	}
}

