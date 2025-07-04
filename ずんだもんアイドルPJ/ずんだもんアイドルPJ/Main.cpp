#include "stdafx.h"

void Main()
{
	Window::SetTitle(U"SHINE VOX");
	Window::Resize(1920, 1080);
	Scene::SetResizeMode(ResizeMode::Keep);
	Window::SetStyle(WindowStyle::Sizable);

	const size_t kMaxCharacters = 5;              // 上限  
	size_t charCount = 0;                         // vvproj 読込で決定  

	// 背景画像
	const Texture background{ U"Texture/background1.jpg" };
	const Texture song_title{ U"Texture/song_title.png" };

	// テキスト表示
	const FilePath fontpath = (U"C:/Program Files/Steinberg/UR-C/font/mplus-1c-medium.ttf");
	const Font font{ FontMethod::MSDF, 48 , fontpath };

	// 時間を管理する変数
	double accumulatedTime = 0;
	bool waitingToPlay = false;
	double waitTimer = 0.0;

	// 伴奏音声
	Audio audio_inst;

	// キャラ用コンテナ（最大数で確保）  
	Array<Texture> characterTex(kMaxCharacters,
		Texture{ U"Texture/Character/ずんだもん（ノーマル）.png" });
	Array<Audio> audios(kMaxCharacters);

	// スピーカー GUI  
	Array<String> Singers;
	Array<int32> SingerIDs;
	for (const auto& spk : VOICEVOX::GetSingers())
		for (const auto& st : spk.styles)
		{
//			if (spk.name == U"ずんだもん") {
			if (st.name == U"ノーマル") {
				Singers << U"{}（{}）"_fmt(spk.name, st.name);
				SingerIDs << st.id;
			}
		}

	Array<ListBoxState> SingerUI(kMaxCharacters, ListBoxState{ Singers });
	Array<Optional<uint64>> prevSel(kMaxCharacters);

	// レイアウト  
	constexpr double kCenterY = 370;
	constexpr Vec2 kListOff{ -150, 180 };

	// 共通パス  
	Optional<FilePath> vvprojPath;
	const FilePath singQuery = U"Query/SingQuery.json";
	const URL queryURL = U"http://localhost:50021/sing_frame_audio_query?Singer=6000";

	for (double i = 0; i > 1.0; i += 0.1) {

		song_title.draw(Arg::center = Scene::Center(), ColorF{ 1.0, 0.5 });
	}

	while (System::Update())
	{
		background.draw(Arg::center = Scene::Center());
		// vvproj 選択  
		if (SimpleGUI::Button(U"🎵 入力ファイルを選択", Vec2{ 1500, 830 }))
		{
			vvprojPath = Dialog::OpenFile({ { U"VOICEVOX Project", { U"vvproj" } } });
			if (vvprojPath)
			{
				charCount = Min(VOICEVOX::GetVVProjTrackCount(*vvprojPath), kMaxCharacters);
			}
		}
		if (charCount == 0) continue;           // 未選択  

		// 横位置を均等割り  
		Array<Vec2> centers;
		const double step = Scene::Width() / (charCount + 1);
		for (size_t i = 0; i < charCount; ++i)
			centers << Vec2{ step * (i + 1), kCenterY };

		// キャラ描画 + ListBox
		for (size_t i = 0; i < charCount; ++i)
		{
			// 音量によってスケールと透明度を決定
			double scale = 0.5;
			double alpha = 0.5;
			if (audios[i].isPlaying())
			{
				FFTResult fft;
				FFT::Analyze(fft, audios[i]);

				// 一部周波数帯域だけのエネルギーを取ることもできるが、今回は全体で
				double energy = std::accumulate(fft.buffer.begin(), fft.buffer.end(), 0.0);
				energy /= fft.buffer.size(); // 平均値

				// デバッグ表示
				//Print << U"キャラ{} energy: {:.5f}"_fmt(i, energy);

				// 閾値は小さめに設定することで反応性アップ
				if (energy > 0.00001)
				{
					scale = 0.55;
					alpha = 1.0;
				}
			}
			characterTex[i].scaled(scale).drawAt(centers[i], ColorF(1.0, alpha));

			SimpleGUI::ListBox(SingerUI[i], centers[i] + kListOff, 300, 220);

			// 選択変更時に立ち絵切替
			if (SingerUI[i].selectedItemIndex != prevSel[i])
			{
				prevSel[i] = SingerUI[i].selectedItemIndex;
				const String sel = Singers[SingerUI[i].selectedItemIndex.value()];
				FilePath tex = U"Texture/Character/" + sel + U".png";
				if (!FileSystem::Exists(tex))
					tex = U"Texture/Character/ずんだもん（ノーマル）.png";	// デフォルトのイラスト
				characterTex[i] = Texture{ tex };
			}
		}

		// -------------- 音声合成 -----------------  
		if (SimpleGUI::Button(U"🎵 音声合成", Vec2{ 1500, 880 }, unspecified, vvprojPath.has_value()))
		{
			const String base = FileSystem::BaseName(*vvprojPath);

			for (size_t i = 0; i < charCount; ++i)
			{
				// vvproj → Score JSON  
				FilePath score = U"Score/" + base + U"_track" + Format(i + 1) + U".json";
				if (!VOICEVOX::ConvertVVProjToScoreJSON(*vvprojPath, score, i))
					continue;

				// Singer ID  
				const uint64 sel = SingerUI[i].selectedItemIndex.value_or(0);
				const int32 spkID = SingerIDs[sel] + 3000;

				const URL synthURL =
					U"http://localhost:50021/frame_synthesis?Singer={}"_fmt(spkID);

				FilePath wav = U"Voice/" + base + U"-" + Singers[sel]
					+ U"_track" + Format(i + 1) + U".wav";

				if (VOICEVOX::SynthesizeFromJSONFileWrapperSplit(
					score, singQuery, wav, queryURL, synthURL, 2500))
				{
					audios[i] = Audio{ wav };
				}

			}
			audio_inst = Audio{ U"Inst/" + base + U".mp3" };
			Console << U"「" + base + U"」の再生準備が完了しました。	";
		}

		// -------------- 再生 ---------------------  
		const bool playable = std::any_of(audios.begin(), audios.begin() + charCount,
										  [](const Audio& a) {return !a.isEmpty(); });


		// ▶️再生ボタン
		if (SimpleGUI::Button(U"▶️再生", Vec2{ 1500, 930 }, unspecified, playable)) {
			waitingToPlay = true;
			waitTimer = 0.0;
		}

		if (waitingToPlay) {
			waitTimer += Scene::DeltaTime();
			const String base = FileSystem::BaseName(*vvprojPath);

			double alpha = 0.0;
			if (waitTimer < 2.0) alpha = waitTimer / 2.0;
			else if (waitTimer < 4.0) alpha = 1.0;
			else if (waitTimer < 6.0) alpha = (6.0 - waitTimer) / 2.0;

			song_title.draw(Arg::center = Scene::Center(), ColorF{ 1.0, alpha });
			font(base).drawAt(45, Scene::Center().x,483, ColorF{ 1.0, alpha });

			if (waitTimer >= 6.0) {
				waitingToPlay = false;

				audio_inst.play();
				audio_inst.setVolume(0.6);
				for (size_t i = 0; i < charCount; ++i)
					if (!audios[i].isEmpty()) audios[i].play();
			}
		}
	}
}
