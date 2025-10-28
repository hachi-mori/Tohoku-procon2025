# include "Scene1.hpp"

Scene1::Scene1(const InitData& init)
	: IScene{ init }
{
	Print << U"===== Scene3: 受け取った結果 =====";

	// 共有データを直接参照して表示
	    for (const auto& t : getData().solvedTasks)
    {
        Print << U"お題：" << t.phrase;
        Print << U"  音節リスト：" << t.syllables; // 例: [え, ぶ, り, で, い]
        Print << U"入力：" << t.userInput;
        Print << U"  入力の音節リスト：" << t.userSyllables;
    }


	// ==== VOICEVOX スピーカー情報の初期化 ====
	for (const auto& spk : VOICEVOX::GetSingers(baseURL))
	{
		if (!spk.styles.isEmpty())
		{
			const auto& st = spk.styles[0]; // 最初のスタイルだけ使用
			SingerLabels << U"{}（{}）"_fmt(spk.name, st.name);
			SingerNames << spk.name;
			StyleNames << st.name;
			SingerIDs << st.id;
		}
	}

	// ListBoxState の初期化（Speakerリストを使って作り直す）
	SingerUI.clear();
	for (size_t i = 0; i < kMaxCharacters; ++i)
	{
		SingerUI << ListBoxState{ SingerLabels };
	}
	prevSel.assign(kMaxCharacters, none);

	// キャラクタ画像を初期化
	characterTex.clear();
	for (size_t i = 0; i < kMaxCharacters; ++i)
	{
		characterTex << Texture{ U"Texture/Character/VOICEVOX/ずんだもん.png" };
	}

	// 音声関係も空で初期化
	songAudio.assign(kMaxCharacters, Audio{});
	talkAudio.assign(kMaxCharacters, Audio{});
	talkStartSecs.assign(kMaxCharacters, 0.0);
	talkPending.assign(kMaxCharacters, false);

	// 簡易フェードイン演出（Main(3) の簡易演出部分）
	for (double i = 0.0; i < 1.0; i += 0.1)
	{
		song_title.draw(Arg::center = Scene::Center(), ColorF{ 1.0, i * 0.5 });
	}

	vvprojPath = getData().vvprojPath;

	if (vvprojPath)
	{
		charCount = Min(VOICEVOX::GetVVProjTrackCount(*vvprojPath), kMaxCharacters);
	}
}

void Scene1::update()
{
	//------------------------------------
	// 🎨 キャラ配置の計算
	//------------------------------------
	Array<Vec2> centers;
	const double step = static_cast<double>(Scene::Width()) / (charCount + 1);

	for (size_t i = 0; i < charCount; ++i)
	{
		centers << Vec2{ step * (i + 1), kCenterY };
	}

	//------------------------------------
	// 🧍 キャラ描画＋リストボックス
	//------------------------------------
	Array<size_t> singingIdx;
	size_t singingNow = 0;

	for (size_t i = 0; i < charCount; ++i)
	{
		// 歌唱アニメ判定
		double scale = 0.8;
		double alpha = 0.5;

		const double eSong = analyzeEnergy(songAudio[i]);
		const double eTalk = analyzeEnergy(talkAudio[i]);
		const double eMax = Max(eSong, eTalk);

		if (eMax > kSingingThreshold)
		{
			++singingNow;
			singingIdx << i;
			scale = 0.9;
			alpha = 1.0;
		}

		characterTex[i].scaled(scale).drawAt(centers[i], ColorF(1.0, alpha));

		SimpleGUI::ListBox(SingerUI[i], centers[i] + kListOff, 300, 220);

		// 選択が変わったらキャラ画像切替
		if (SingerUI[i].selectedItemIndex != prevSel[i])
		{
			prevSel[i] = SingerUI[i].selectedItemIndex;
			const String selLabel = SingerLabels[SingerUI[i].selectedItemIndex.value()];
			const FilePath charFolder = U"Texture/Character/VOICEVOX/";
			const auto files = FileSystem::DirectoryContents(charFolder, Recursive::No);

			Optional<FilePath> matchedTex;
			for (const auto& f : files)
			{
				const String fileName = FileSystem::BaseName(f);
				if (selLabel.starts_with(fileName))
				{
					matchedTex = f;
					break;
				}
			}

			FilePath tex = matchedTex.value_or(charFolder + U"ずんだもん.png");
			characterTex[i] = Texture{ tex };
		}

		// 音量とパンの表示
		double volume = songAudio[i].getVolume();
		font(U"Vol: {:.2f}"_fmt(volume)).drawAt(centers[i] + Vec2{ 0, 150 }, ColorF{ 1.0 });

		double pan = songAudio[i].getPan();
		font(U"Pan: {:+.2f}"_fmt(pan)).drawAt(centers[i] + Vec2{ 0, 110 }, ColorF{ 1.0 });
	}

	//------------------------------------
	// 🎶 音声合成ボタン
	//------------------------------------
	if (SimpleGUI::Button(U"🎵 音声合成", Vec2{ 1500, 880 }, unspecified, vvprojPath.has_value()))
	{
		const String base = FileSystem::BaseName(*vvprojPath);
		getData().SingingNames.clear();  // ← 初期化しておく

		for (size_t i = 0; i < charCount; ++i)
		{
			const uint64 selIdx = SingerUI[i].selectedItemIndex.value_or(0);
			const String singerName = SingerNames[selIdx];
			getData().SingingNames << singerName; 

			const int32  spkID = SingerIDs[selIdx];
			const int32  talkSpkID = spkID - 3000;

			FilePath songwav = U"Voice/" + base + U"-" + SingerLabels[selIdx] + U"_track" + Format(i + 1) + U".wav";
			FilePath talkwav = U"Voice/" + base + U"-" + SingerLabels[selIdx] + U"_talk_track" + Format(i + 1) + U".wav";

			FilePath score = U"tmp/tmp_" + base + U"_track" + Format(i + 1) + U".json";

			if (!VOICEVOX::ConvertVVProjToScoreJSON(*vvprojPath, score, i))
				continue;

			FilePath talkOut = U"tmp/tmp_talk_" + base + U"_track" + Format(i + 1) + U".json";
			double talkStartSec = 0.0;
			const bool talkOk = VOICEVOX::ConvertVVProjToTalkQueryJSON(
				baseURL, *vvprojPath, talkOut, talkSpkID, &talkStartSec, i + charCount);

			talkStartSecs[i] = Max(0.0, talkStartSec - 0.155);

			int keyShift = VOICEVOX::GetKeyAdjustment(SingerNames[selIdx], StyleNames[selIdx]);
			if (VOICEVOX::SynthesizeFromJSONFileWrapperSplit(score, songwav, spkID, baseURL, 2500, keyShift))
			{
				songAudio[i] = Audio{ songwav, Loop::Yes };
				FileSystem::Remove(score);
			}

			if (talkOk)
			{
				const FilePath talkPrefix = U"Voice/" + base + U"-" + SingerLabels[selIdx] + U"_talk_track" + Format(i + 1);
				const FilePath joinedTalk = talkPrefix + U"_joined.wav";

				if (VOICEVOX::SynthesizeFromVVProjWrapperSplitTalkJoin(
					baseURL, *vvprojPath, talkPrefix, joinedTalk, talkSpkID, i + charCount, 3750))
				{
					if (FileSystem::Exists(joinedTalk))
					{
						talkAudio[i] = Audio{ joinedTalk };
					}
				}
			}

			Console << U"{}（{}） → Shift:{}"_fmt(SingerNames[selIdx], StyleNames[selIdx],
				VOICEVOX::GetKeyAdjustment(SingerNames[selIdx], StyleNames[selIdx]));
		}

		audio_inst = Audio{ U"Inst/" + base + U".mp3", Loop::Yes };
		Console << U"「" + base + U"」の再生準備が完了しました。\t";
	}

	//------------------------------------
	// ▶️ 再生ボタン
	//------------------------------------
	bool playable = std::any_of(songAudio.begin(), songAudio.begin() + charCount,
		[](const Audio& a) { return !a.isEmpty(); });

	if (SimpleGUI::Button(U"▶️再生", Vec2{ 1500, 930 }, unspecified, playable))
	{
		changeScene(U"Scene2");
		// 共有データへの保存
		getData().charCount = charCount;
		getData().SingerNames = SingerNames;
		getData().StyleNames = StyleNames;
		getData().characterTex = characterTex;
		getData().songAudio = songAudio;
		getData().talkAudio = talkAudio;
		getData().instAudio = audio_inst;
		getData().vvprojPath = *vvprojPath;
		getData().songTitle = FileSystem::BaseName(*vvprojPath);
		getData().talkStartSecs = talkStartSecs;
		getData().talkPending = talkPending;
		getData().readyToPlay = true;

		waitingToPlay = true;
		waitTimer = 0.0;
	}

}

void Scene1::draw() const
{
	// ① 背景（最初に描画）
	background.draw();

	// ③ キャラクター立ち絵の描画
	const double step = static_cast<double>(Scene::Width()) / (charCount + 1);
	Array<Vec2> centers;

	for (size_t i = 0; i < charCount; ++i)
	{
		centers << Vec2{ step * (i + 1), kCenterY };
	}

	for (size_t i = 0; i < charCount; ++i)
	{
		// キャラ画像
		characterTex[i].drawAt(centers[i], ColorF{ 1.0 });
	}

	// ⑤ 各キャラごとの ListBox（UIの一部）
	for (size_t i = 0; i < charCount; ++i)
	{
		SimpleGUI::ListBox(SingerUI[i], centers[i] + kListOff, 300, 220);
	}

	//  ⑥ GUI部品（最前面・右側操作系）
	//SimpleGUI::Button(U"🎵 入力ファイルを選択", Vec2{ 1500, 830 });
	SimpleGUI::Button(U"🎵 音声合成", Vec2{ 1500, 880 }, unspecified, vvprojPath.has_value());
	SimpleGUI::Button(U"▶️再生", Vec2{ 1500, 930 });

}


double Scene1::analyzeEnergy(const Audio& a) const
{
	if (!a.isPlaying())
	{
		return 0.0;
	}

	FFTResult fft;
	FFT::Analyze(fft, a);

	if (fft.buffer.empty())
	{
		return 0.0;
	}

	double energy = std::accumulate(fft.buffer.begin(), fft.buffer.end(), 0.0);
	energy /= fft.buffer.size();
	return energy;
}
