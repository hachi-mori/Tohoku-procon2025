# include "../stdafx.h"

namespace VOICEVOX
{
	// 共有ユーティリティ
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

	// 音域調整テーブル（キャラ名 → スタイル名 → 音域調整値）
	static const HashTable<String, HashTable<String, int32>> kKeyAdjustmentTable = {
		{ U"四国めたん",    {{U"ノーマル",-4},{U"あまあま",-4},{U"ツンツン",-5},{U"セクシー",-4},{U"ヒソヒソ",-9}} },
		{ U"ずんだもん",    {{U"ノーマル",-2},{U"あまあま",0},{U"ツンツン",-3},{U"セクシー",0},{U"ヒソヒソ",-7},{U"ヘロヘロ",-3},{U"なみだめ",6}} },
		{ U"春日部つむぎ",  {{U"ノーマル",-2}} },
		{ U"雨晴はう",      {{U"ノーマル",0}} },
		{ U"波音リツ",      {{U"ノーマル",-8},{U"クイーン",-5}} },
		{ U"玄野武宏",      {{U"ノーマル",-17},{U"喜び",-9},{U"ツンギレ",-14},{U"悲しみ",-18}} },
		{ U"白上虎太郎",    {{U"ふつう",-14},{U"わーい",-8},{U"びくびく",-7},{U"おこ",-9},{U"びえーん",-3}} },
		{ U"青山龍星",      {{U"ノーマル",-22},{U"熱血",-18},{U"不機嫌",-23},{U"喜び",-21},{U"しっとり",-27},{U"かなしみ",-22}} },
		{ U"冥鳴ひまり",    {{U"ノーマル",-7}} },
		{ U"九州そら",      {{U"ノーマル",-7},{U"あまあま",-2},{U"ツンツン",-6},{U"セクシー",-4}} },
		{ U"もち子さん",    {{U"ノーマル",-5},{U"セクシー／あん子",-7},{U"泣き",-2},{U"怒り",-3},{U"喜び",-2},{U"のんびり",-5}} },
		{ U"剣崎雌雄",      {{U"ノーマル",-18}} },
		{ U"WhiteCUL",      {{U"ノーマル",-6},{U"たのしい",-3},{U"かなしい",-7},{U"びえーん",0}} },
		{ U"後鬼",          {{U"人間ver.",-7},{U"ぬいぐるみver.",-2}} },
		{ U"No.7",         {{U"ノーマル",-8},{U"アナウンス",-10},{U"読み聞かせ",-9}} },
		{ U"ちび式じい",    {{U"ノーマル",-18}} },
		{ U"櫻歌ミコ",      {{U"ノーマル",-6},{U"第二形態",-12},{U"ロリ",-7}} },
		{ U"小夜/SAYO",     {{U"ノーマル",-4}} },
		{ U"ナースロボ＿タイプＴ", {{U"ノーマル",-6},{U"楽々",-3},{U"恐怖",-4}} },
		{ U"†聖騎士 紅桜†", {{U"ノーマル",-15}} },
		{ U"雀松朱司",      {{U"ノーマル",-21}} },
		{ U"麒ヶ島宗麟",    {{U"ノーマル",-17}} },
		{ U"春歌ナナ",      {{U"ノーマル",-2}} },
		{ U"猫使アル",      {{U"ノーマル",-8},{U"おちつき",-9},{U"うきうき",-7}} },
		{ U"猫使ビィ",      {{U"ノーマル",-1},{U"おちつき",-3}} },
		{ U"中国うさぎ",    {{U"ノーマル",-8},{U"おどろき",-4},{U"こわがり",-2},{U"へろへろ",-4}} },
		{ U"栗田まろん",    {{U"ノーマル",-14}} },
		{ U"あいえるたん",  {{U"ノーマル",-2}} },
		{ U"満別花丸",      {{U"ノーマル",-4},{U"元気",2},{U"ささやき",-33},{U"ぶりっ子",0},{U"ボーイ",-10}} },
		{ U"琴詠ニア",      {{U"ノーマル",-4}} },
	};


	int32 GetKeyAdjustment(const String& singer,
							const String& style)
	{
		auto itSinger = kKeyAdjustmentTable.find(singer);
		if (itSinger != kKeyAdjustmentTable.end()) {
			const auto& styleTable = itSinger->second;

			auto itStyle = styleTable.find(style);
			if (itStyle != styleTable.end()) {
				return itStyle->second;
			}
		}
		return 0;
	}

	
	bool TransposeScoreJSON(const FilePath& inPath,
							const FilePath& outPath,
							int semitone)
	{
		JSON score = JSON::Load(inPath);
		if (!score || !score.contains(U"notes") || !score[U"notes"].isArray())
			return false;

		const size_t noteCount = score[U"notes"].size();

		for (size_t i = 0; i < noteCount; ++i)
		{
			JSON n = score[U"notes"][i];                 // 値で取得（コピー）

			if (auto midiOpt = n[U"key"].getOpt<int32>())
			{
				int32 midi = Clamp<int32>(*midiOpt + semitone, 0, 127);
				n[U"key"] = midi;
				n[U"notelen"] = MidiToName(midi);
			}
			score[U"notes"][i] = n;                      // 書き戻し
		}
		return score.save(outPath);
	}

	
	bool TransposeSingQueryJSON(const FilePath& inPath,
								const FilePath& outPath,
								int semitone)
	{
		JSON q = JSON::Load(inPath);
		if (!q) return false;

		const double ratio = std::pow(2.0, semitone / 12.0);

		/* --- f0 カーブ --- */
		if (q.contains(U"f0") && q[U"f0"].isArray())
		{
			const size_t f0Count = q[U"f0"].size();
			for (size_t i = 0; i < f0Count; ++i)
			{
				if (auto d = q[U"f0"][i].getOpt<double>())
				{
					q[U"f0"][i] = (*d) * ratio;         // 直接上書き
				}
			}
		}

		/* --- phonemes.note_id --- */
		if (q.contains(U"phonemes") && q[U"phonemes"].isArray())
		{
			const size_t phCount = q[U"phonemes"].size();
			for (size_t i = 0; i < phCount; ++i)
			{
				JSON p = q[U"phonemes"][i];             // コピー
				if (auto nOpt = p[U"note_id"].getOpt<int32>())
				{
					p[U"note_id"] = *nOpt + semitone;
				}
				q[U"phonemes"][i] = p;                  // 書き戻し
			}
		}
		return q.save(outPath);
	}

	
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

	[[nodiscard]]
	bool SynthesizeFromJSONFile(const FilePath& jsonFilePath,
								const FilePath& savePath,
								const URL& synthesisURL,
								const Duration timeout)
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

				Console(U"リクエストがタイムアウトしました: " + savePath);
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

			Console(U"リクエストが失敗しました: " + savePath);
			return false;
		}

		Console(U"ファイルが保存されました: " + savePath);
		return true;
	}
	
	[[nodiscard]]
	bool VOICEVOX::SynthesizeFromJSONFileWrapperSplit(
		const FilePath& inputPath,          // ScoreQuery.json
		const FilePath& intermediatePath,   // 使わないが互換のため残す
		const FilePath& outputPath,         // 出力 WAV
		const URL& queryURL,           // /sing_frame_audio_query
		const URL& synthesisURL,       // /frame_synthesis
		size_t          maxFrames,
		int             keyShift)
	{
		//──────────────────── ⓪ 条件付きで、Score をオク下にする ────────────────────
		const int octave = 12;
		if (keyShift < -12) // 基準値以下はオク下
		{	
			if (!VOICEVOX::TransposeScoreJSON(inputPath, inputPath, -octave)) // -12でオク下になる
			{
				Console(U"Score 移調に失敗しました");
				return false;
			}
		}
		if (keyShift < -20) // 基準値以下はさらにオク下
		{
			if (!VOICEVOX::TransposeScoreJSON(inputPath, inputPath, -octave)) // -12でオク下になる
			{
				Console(U"Score 移調に失敗しました");
				return false;
			}
		}

		//──────────────────── ① Score を移調（+方向 = -keyShift） ────────────────────
		if (keyShift != 0)
		{
			// 例: keyShift = -3 → Score を +3 半音上げる
			if (!VOICEVOX::TransposeScoreJSON(inputPath, inputPath, -keyShift))
			{
				Console(U"Score 移調に失敗しました");
				return false;
			}
		}

		//──────────────────── ② Score JSON 読込 & notes 分割 ────────────────────────
		JSON score = JSON::Load(inputPath);
		if (!score)
		{
			Console(U"Score JSON の読み込み失敗");
			return false;
		}

		const auto& notes = score[U"notes"];
		if (!notes.isArray())
		{
			Console(U"notes が配列ではありません");
			return false;
		}

		Array<JSON> segments;
		Array<JSON> currentSegment;
		size_t frameSum = 0;
		Optional<JSON> carriedOverRest;

		for (const auto& note : notes.arrayView())
		{
			currentSegment << note;
			frameSum += note[U"frame_length"].get<size_t>();
			const bool isRest = (note[U"notelen"].getString() == U"R");

			if (frameSum >= maxFrames && isRest)
			{
				// ─ 休符を 2 分割してセグメント境界にする ─
				const size_t restFrame = note[U"frame_length"].get<size_t>();
				const size_t half1 = restFrame / 2;
				const size_t half2 = restFrame - half1;

				if (!currentSegment.isEmpty())
				{
					currentSegment.pop_back(); // 元休符を除去

					JSON rest1;
					rest1[U"frame_length"] = half1;
					rest1[U"key"] = JSON();
					rest1[U"lyric"] = U"";
					rest1[U"notelen"] = U"R";
					currentSegment << rest1;
				}

				JSON rest2;
				rest2[U"frame_length"] = half2;
				rest2[U"key"] = JSON();
				rest2[U"lyric"] = U"";
				rest2[U"notelen"] = U"R";
				carriedOverRest = rest2;

				JSON segJson;
				segJson[U"notes"] = currentSegment;
				segments << segJson;

				// リセット
				currentSegment.clear();
				frameSum = 0;

				currentSegment << *carriedOverRest;
			}
		}

		if (!currentSegment.isEmpty())
		{
			JSON segJson;
			segJson[U"notes"] = currentSegment;
			segments << segJson;
		}

		//──────────────────── ③ 各セグメントで合成 ───────────────────────────────
		Array<FilePath> tempWavs;
		for (size_t i = 0; i < segments.size(); ++i)
		{
			// 一時ファイル群
			const FilePath tmpScore = U"tmp/tmp_score_" + Format(i) + U".json";
			const FilePath tmpQuery = U"tmp/tmp_query_" + Format(i) + U".json";
			const FilePath tmpWav = U"tmp/tmp_part_" + Format(i) + U".wav";
			segments[i].save(tmpScore);

			// ScoreQuery → SingQuery
			if (!VOICEVOX::SynthesizeFromJSONFile(tmpScore, tmpQuery, queryURL))
			{
				Console(U"SingQuery 作成失敗 (分割 " + Format(i) + U")");
				return false;
			}

			//──────────────── ④ SingQuery を -keyShift 半音戻す ────────────────
			if (keyShift != 0 &&
				!VOICEVOX::TransposeSingQueryJSON(tmpQuery, tmpQuery, keyShift))
			{
				Console(U"SingQuery 移調に失敗しました");
				return false;
			}

			// 音量などのメタ調整
			if (JSON query = JSON::Load(tmpQuery))
			{
				query[U"volumeScale"] = 1.0;
				query[U"outputSamplingRate"] = 44100;
				query[U"outputStereo"] = true;
				query.save(tmpQuery);
			}

			// SingQuery → WAV
			if (!VOICEVOX::SynthesizeFromJSONFile(tmpQuery, tmpWav, synthesisURL))
			{
				Console(U"音声合成失敗 (分割 " + Format(i) + U")");
				return false;
			}
			tempWavs << tmpWav;
		}

		//──────────────────── ⑤ 分割 WAV を連結 ──────────────────────────────
		Wave joined;
		for (const auto& wav : tempWavs)
		{
			Wave part{ wav };
			joined.append(part);
		}
		joined.save(outputPath);

		//──────────────────── ⑥ 一時ファイルを掃除 ────────────────────────────
		for (size_t i = 0; i < segments.size(); ++i)
		{
			FileSystem::Remove(U"tmp/tmp_score_" + Format(i) + U".json");
			FileSystem::Remove(U"tmp/tmp_query_" + Format(i) + U".json");
			FileSystem::Remove(U"tmp/tmp_part_" + Format(i) + U".wav");
		}

		Console(U"分割合成＆連結が完了: " + outputPath);
		return true;
	}

	[[nodiscard]]
	JSON CreateQuery(const String text, const int32 speakerID,
		const double intonationScale, const double speedScale, const double volumeScale, const double pitchScale,
		const Duration timeout)
	{
		const URL url = U"http://localhost:50021/audio_query?text={}&speaker={}"_fmt(PercentEncode(text), speakerID);
		const std::string data = JSON{}.formatUTF8Minimum();
		AsyncHTTPTask task = SimpleHTTP::PostAsync(url, {}, data.data(), data.size());
		Stopwatch stopwatch{ StartImmediately::Yes };

		while (not task.isReady())
		{
			if (timeout <= stopwatch)
			{
				task.cancel();
				return JSON::Invalid();
			}

			System::Sleep(1ms);
		}

		if (not task.getResponse().isOK())
		{
			return JSON::Invalid();
		}

		JSON query = task.getAsJSON();
		query[U"intonationScale"] = intonationScale;
		query[U"speedScale"] = speedScale;
		query[U"volumeScale"] = volumeScale;
		query[U"pitchScale"] = pitchScale;

		return query;
	}

	[[nodiscard]]
	bool ConvertVVProjToTalkQueryJSON(const FilePath& vvprojPath,
									  const FilePath& outJsonPath,
									  const int32 speakerID, double* outTalkStartSec)
	{
		// ① vvproj（JSON）を読み込む
		const JSON src = JSON::Load(vvprojPath);
		if (!src || !src.contains(U"talk"))
		{
			return false;
		}

		// --- 追加: song から talk 開始秒を算出 ---
		double talkStartSecLocal = 0.0;

		if (src.contains(U"song") && src[U"song"].isObject())
		{
			const JSON& song = src[U"song"];

			// tpqn
			const double tpqn = song[U"tpqn"].isNumber()
				? song[U"tpqn"].get<double>() : 480.0;

			// bpm（VOICEVOX.cpp のユーティリティを流用）
			const double bpm = VOICEVOX::GetFirstBPM(song);  // 先頭テンポを使用 :contentReference[oaicite:1]{index=1}

			// 全トラックから最初のノート position（最小値）を探す
			Optional<int64> earliestTick;

			if (song.contains(U"tracks") && song[U"tracks"].isObject())
			{
				for (auto&& [__, track] : song[U"tracks"])
				{
					if (!track[U"notes"].isArray()) continue;

					for (auto&& n : track[U"notes"].arrayView())
					{
						if (!n[U"position"].isNumber()) continue;
						const int64 pos = n[U"position"].get<int64>();

						if (!earliestTick || pos < *earliestTick)
							earliestTick = pos;
					}
				}
			}

			// ticks -> 秒（beats = ticks/tpqn, seconds = beats*(60/BPM)）
			if (earliestTick)
			{
				const double beats = static_cast<double>(*earliestTick) / tpqn;
				talkStartSecLocal = beats * (60.0 / bpm);
			}
			else
			{
				talkStartSecLocal = 0.0; // ノートが無い場合は 0 秒
			}
		}

		// 呼び出し元へ返す（nullptrなら何もしない）
		if (outTalkStartSec) *outTalkStartSec = talkStartSecLocal;

		// ② talk セクションを参照し、audioKeys の先頭キーを取得
		const JSON& talk = src[U"talk"];
		if (!talk.contains(U"audioKeys") || !talk[U"audioKeys"].isArray())
		{
			return false;
		}
		const auto keys = talk[U"audioKeys"].arrayView();
		if (keys.begin() == keys.end())
		{
			return false; // 空配列
		}
		const String key = (*keys.begin()).getString();

		// ③ 該当 audioItem から text を取り出す
		if (!talk.contains(U"audioItems") || !talk[U"audioItems"][key].isObject())
		{
			return false;
		}
		const JSON& item = talk[U"audioItems"][key];
		const String text = item[U"text"].getOr<String>(U"");

		// ④ /audio_query に投げてベースクエリを取得
		JSON query = CreateQuery(
			text, speakerID,
			/*intonationScale*/ 1.0,
			/*speedScale*/      1.0,
			/*volumeScale*/     1.0,
			/*pitchScale*/      0.0,
			SecondsF{ 5.0 });


		if (!query)
		{
			return false;
		}

		// ⑤’ vvproj 内の talk.query.accentPhrases[].moras[].{vowelLength,consonantLength}
		//     をベースクエリの accent_phrases[].moras[].{vowel_length,consonant_length} に反映
		do
		{
			// vvproj 側の query が無い・形式不一致なら何もせず保存へ（安全にスキップ）
			if (!item.contains(U"query") || !item[U"query"].isObject())
				break;

			const JSON& srcQuery = item[U"query"];
			if (!srcQuery.contains(U"accentPhrases") || !srcQuery[U"accentPhrases"].isArray())
				break;

			// ベースクエリ側もチェック
			if (!query.contains(U"accent_phrases") || !query[U"accent_phrases"].isArray())
				break;

			const size_t apCount = Min(srcQuery[U"accentPhrases"].size(),
									   query[U"accent_phrases"].size());

			for (size_t i = 0; i < apCount; ++i)
			{
				const JSON& srcAP = srcQuery[U"accentPhrases"][i];
				JSON        dstAP = query[U"accent_phrases"][i]; // コピー（あとで書き戻す）

				if (!srcAP.contains(U"moras") || !srcAP[U"moras"].isArray())
					continue;

				if (!dstAP.contains(U"moras") || !dstAP[U"moras"].isArray())
					continue;

				const size_t moraCount = Min(srcAP[U"moras"].size(), dstAP[U"moras"].size());

				for (size_t j = 0; j < moraCount; ++j)
				{
					const JSON& srcMora = srcAP[U"moras"][j];
					JSON        dstMora = dstAP[U"moras"][j]; // コピー

					// vowelLength -> vowel_length
					if (auto vlen = srcMora[U"vowelLength"].getOpt<double>())
					{
						dstMora[U"vowel_length"] = *vlen;
					}
					// consonantLength -> consonant_length（存在する場合のみ）
					if (auto clen = srcMora[U"consonantLength"].getOpt<double>())
					{
						dstMora[U"consonant_length"] = *clen;
					}

					// 1モーラ書き戻し
					dstAP[U"moras"][j] = dstMora;
				}

				/* --- 読点（pause）を反映 --- */
				if (srcAP.contains(U"pauseMora") && srcAP[U"pauseMora"].isObject())
				{
					const JSON& sp = srcAP[U"pauseMora"];

					JSON dp; // dst pause_mora
					// JSON null を入れたいフィールドは JSON{} ではなく JSON() を代入
					dp[U"consonant"] = JSON();  // null
					dp[U"consonant_length"] = JSON();  // null

					// テキストは vvproj の値を優先（無ければ "、"）
					dp[U"text"] = sp[U"text"].getOr<String>(U"、");

					// 母音は pau 固定（vvproj 側の値があればそれを尊重）
					dp[U"vowel"] = sp[U"vowel"].getOr<String>(U"pau");

					// 長さは camelCase → snake_case へ（無ければ 0）
					dp[U"vowel_length"] = sp[U"vowelLength"].getOr<double>(0.0);

					// ピッチは 0 を基本（vvproj 側に数値があればそれ）
					dp[U"pitch"] = sp[U"pitch"].getOr<double>(0.0);

					// 書き戻し（無条件で上書き）
					dstAP[U"pause_mora"] = dp;
				}
				else
				{
					// 読点無し：null を明示して統一
					dstAP[U"pause_mora"] = JSON(); // null
				}

				// 1フレーズ書き戻し
				query[U"accent_phrases"][i] = dstAP;
			}
		} while (false);

		// ⑥ 取得したクエリを保存
		return query.save(outJsonPath);
	}
}
