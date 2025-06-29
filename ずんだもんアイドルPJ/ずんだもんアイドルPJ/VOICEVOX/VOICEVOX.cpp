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

	// 分割ラッパー（maxFramesはデフォルト5000、必要なら上書き可）
	[[nodiscard]]
	bool VOICEVOX::SynthesizeFromJSONFileWrapperSplit(
	const FilePath& inputPath,
	const FilePath& intermediatePath,
	const FilePath& outputPath,
	const URL& queryURL,
	const URL& synthesisURL,
	size_t maxFrames)
	{
		// JSON読込
		JSON score = JSON::Load(inputPath);
		if (!score) {
			Console(U"Score JSONの読み込み失敗");
			return false;
		}

		// notes配列取得
		const auto& notes = score[U"notes"];
		if (!notes.isArray()) {
			Console(U"notesが配列じゃない");
			return false;
		}

		// 分割
		Array<JSON> segments;
		Array<JSON> currentSegment;
		size_t frameSum = 0;
		Optional<JSON> carriedOverRest;

		for (const auto& note : notes.arrayView()) {
			currentSegment << note;
			frameSum += note[U"frame_length"].get<size_t>();
			bool isRest = (note[U"notelen"].getString() == U"R");

			if (frameSum >= maxFrames && isRest) {
				// この休符のframe数を取得
				const size_t restFrame = note[U"frame_length"].get<size_t>();
				const size_t half1 = restFrame / 2;
				const size_t half2 = restFrame - half1;

				// 直前のセグメントの末尾に half1 を入れる
				if (!currentSegment.isEmpty()) {
					currentSegment.pop_back(); // この休符は一旦除去

					JSON rest1;
					rest1[U"frame_length"] = half1;
					rest1[U"key"] = JSON();
					rest1[U"lyric"] = U"";
					rest1[U"notelen"] = U"R";
					currentSegment << rest1;
				}

				// 休符の後半は次のセグメントの先頭に持ち越し
				JSON rest2;
				rest2[U"frame_length"] = half2;
				rest2[U"key"] = JSON();
				rest2[U"lyric"] = U"";
				rest2[U"notelen"] = U"R";
				carriedOverRest = rest2;

				// セグメントとして保存
				JSON segJson;
				segJson[U"notes"] = currentSegment;
				segments << segJson;

				// 初期化
				currentSegment.clear();
				frameSum = 0;

				// 後半休符を次セグメントに挿入
				currentSegment << *carriedOverRest;
			}
		}

		// 残りを追加
		if (!currentSegment.isEmpty()) {
			JSON segJson;
			segJson[U"notes"] = currentSegment;
			segments << segJson;
		}

		// 各セグメントで合成
		Array<FilePath> tempWavs;
		for (size_t i = 0; i < segments.size(); ++i) {
			// 一時jsonファイル
			FilePath tmpScore = U"tmp_score_" + Format(i) + U".json";
			FilePath tmpQuery = U"tmp_query_" + Format(i) + U".json";
			FilePath tmpWav = U"tmp_part_" + Format(i) + U".wav";
			segments[i].save(tmpScore);

			// ScoreQuery -> SingQuery
			if (!VOICEVOX::SynthesizeFromJSONFile(tmpScore, tmpQuery, queryURL)) {
				Console(U"SingQuery作成失敗(分割" + Format(i) + U")");
				return false;
			}

			// SingQuery を作った直後に JSON を読み込んで修正
			JSON query = JSON::Load(tmpQuery);
			if (query) {
				query[U"volumeScale"] = 1.0;              // 音量スケール（0.0〜1.0推奨）
				query[U"outputSamplingRate"] = 44100;     // 出力レート
				query[U"outputStereo"] = true;            // ステレオ出力ON
				query.save(tmpQuery);                     // 上書き保存
			}
			else {
				Console << U"SingQuery JSON の読み込みに失敗しました";
				return false;
			}

			// SingQuery -> WAV
			if (!VOICEVOX::SynthesizeFromJSONFile(tmpQuery, tmpWav, synthesisURL)) {
				Console(U"音声合成失敗(分割" + Format(i) + U")");
				return false;
			}
			tempWavs << tmpWav;
			Wave part{ tmpWav };
			Console << part.sampleRate(); // 各WAVのサンプリングレート
		}

		// 分割WAV連結
		Wave joined;
		for (const auto& wav : tempWavs) {
			Wave part{ wav };
			joined.append(part);
		}
		joined.save(outputPath);

		// 一時ファイル削除
		for (size_t i = 0; i < segments.size(); ++i) {
			FileSystem::Remove(U"tmp_score_" + Format(i) + U".json");
			FileSystem::Remove(U"tmp_query_" + Format(i) + U".json");
			FileSystem::Remove(U"tmp_part_" + Format(i) + U".wav");
		}

		Console(U"分割合成＆連結が完了：" + outputPath);
		return true;
	}
}
