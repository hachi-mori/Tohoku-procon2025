# include "../stdafx.h"

namespace VOICEVOX
{
	// -----------------------------------------------------------------------------
	// 1. 共有ユーティリティ
	// -----------------------------------------------------------------------------
	namespace
	{
		constexpr double kFrameRate = 93.75;

		String MidiToName(int midi)
		{
			static const String names[12] =
			{ U"C", U"C#", U"D", U"D#", U"E", U"F",
			  U"F#", U"G", U"G#", U"A", U"A#", U"B" };

			const int octave = midi / 12 - 1;
			return names[midi % 12] + Format(octave);
		}

		int CalcFrameLen(int64 ticks, double bpm, double tpqn, double& carry)
		{
			const double beats = ticks / tpqn;
			const double sec = beats * (60.0 / bpm);
			const double frames = sec * kFrameRate + carry;

			const int   flen = Max(1, static_cast<int>(std::floor(frames + 0.5)));
			carry = frames - flen;        // 誤差キャリー
			return flen;
		}

		double GetFirstBPM(const JSON& song)
		{
			if (song[U"tempos"].isArray()
			 && (song[U"tempos"].arrayView().begin() != song[U"tempos"].arrayView().end())
			 && song[U"tempos"][0][U"bpm"].isNumber())
			{
				return song[U"tempos"][0][U"bpm"].get<double>();
			}
			return 120.0;  // デフォルト
		}
	} // unnamed-namespace

	// -----------------------------------------------------------------------------
	// 2. HTTP: Singer 一覧
	// -----------------------------------------------------------------------------
	Array<Singer> GetSingers(const Duration timeout)
	{
		constexpr URLView url = U"http://localhost:50021/singers";

		AsyncHTTPTask task = SimpleHTTP::GetAsync(url, {});
		Stopwatch     sw{ StartImmediately::Yes };

		while (!task.isReady())
		{
			if (timeout <= sw)
			{
				task.cancel();
				return{};
			}
			System::Sleep(1ms);
		}
		if (!task.getResponse().isOK())
			return{};

		const JSON json = task.getAsJSON();
		Array<Singer> out;

		for (auto&& [_, sp] : json)
		{
			Singer s;  s.name = sp[U"name"].get<String>();

			for (auto&& [__, st] : sp[U"styles"])
			{
				Singer::Style tmp;
				tmp.name = st[U"name"].get<String>();
				tmp.id = st[U"id"].get<int32>();
				s.styles << tmp;
			}
			out << s;
		}
		return out;
	}

	// -----------------------------------------------------------------------------
	// 3. vvproj ヘルパ
	// -----------------------------------------------------------------------------
	size_t GetVVProjTrackCount(const FilePath& vvprojPath)
	{
		const JSON src = JSON::Load(vvprojPath);
		const JSON& song = src[U"song"];

		if (!song || !song[U"tracks"].isObject())
			return 0;

		if (song[U"trackOrder"].isArray())
			return song[U"trackOrder"].size();

		return song[U"tracks"].size();
	}

	// vvproj → Score JSON（指定トラックだけ）
	bool ConvertVVProjToScoreJSON(const FilePath& vvprojPath,
								  const FilePath& outJsonPath,
								  size_t trackIndex)
	{
		const JSON src = JSON::Load(vvprojPath);
		const JSON& song = src[U"song"];

		if (!song || !song[U"tracks"].isObject())
			return false;

		// ---------------------------------- ① 目的トラック抽出
		JSON track;
		bool found = false;

		if (song[U"trackOrder"].isArray() &&
			trackIndex < song[U"trackOrder"].size())
		{
			const String key = song[U"trackOrder"][trackIndex].getString();
			if (song[U"tracks"][key].isObject())
			{
				track = song[U"tracks"][key];
				found = true;
			}
		}
		if (!found)                                   // trackOrder 無し fallback
		{
			size_t cur = 0;
			for (auto&& [__, tr] : song[U"tracks"])
			{
				if (cur == trackIndex)
				{
					track = tr;
					found = true;
					break;
				}
				++cur;
			}
		}
		if (!found || !track[U"notes"].isArray())
			return false;

		// ---------------------------------- ② スコア変換
		const double tpqn = song[U"tpqn"].isNumber()
			? song[U"tpqn"].get<double>() : 480.0;
		const double bpm = GetFirstBPM(song);

		Array<JSON> outNotes;
		double carry = 0.0;

		auto putRest = [&](int f)
			{
				JSON r;
				r[U"frame_length"] = f;
				r[U"key"] = JSON();
				r[U"lyric"] = U"";
				r[U"notelen"] = U"R";
				outNotes << r;
			};
		putRest(2);                                   // 開頭休符 2frame

		int64 prevEnd = 0;
		for (auto&& note : track[U"notes"].arrayView())
		{
			const int64 pos = note[U"position"].get<int64>();
			const int64 dur = note[U"duration"].get<int64>();
			const int   midi = note[U"noteNumber"].get<int>();
			const String lyr = note[U"lyric"].getString();

			if (const int64 gap = pos - prevEnd; gap > 0)
				putRest(CalcFrameLen(gap, bpm, tpqn, carry));

			JSON n;
			n[U"frame_length"] = CalcFrameLen(dur, bpm, tpqn, carry);
			n[U"key"] = midi;
			n[U"lyric"] = lyr;
			n[U"notelen"] = MidiToName(midi);
			outNotes << n;

			prevEnd = pos + dur;
		}
		putRest(2);                                   // 終端休符

		JSON res; res[U"notes"] = outNotes;
		res.save(outJsonPath);
		return true;
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
			FilePath tmpScore = U"tmp/tmp_score_" + Format(i) + U".json";
			FilePath tmpQuery = U"tmp/tmp_query_" + Format(i) + U".json";
			FilePath tmpWav = U"tmp/tmp_part_" + Format(i) + U".wav";
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
			//Console << part.sampleRate(); // 各WAVのサンプリングレート
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
