# include "stdafx.h"

void Main()
{

	Window::SetTitle(U"SHINE VOX");
	Window::Resize(1920, 1080);
	Scene::SetResizeMode(ResizeMode::Keep);
	Window::SetStyle(WindowStyle::Sizable);
	//Window::SetFullscreen(true);
	//Scene::SetBackground(Color{ 226, 0, 128 });

	// 背景画像
	const Texture background1 = Texture{ U"Texture/background1.jpg" };
	// キャラクター画像
	Texture characterTexture1 = Texture{ U"Texture/Character/ずんだもん（ノーマル）.png" };

	// 音声ファイルの読み込み
	Audio audio1;

	// ファイルパス
	Optional<FilePath> inputpath;

	// 出力ファイルのパス
	const FilePath singQueryFilePath = U"Query/SingQuery.json";
	FilePath outputAudioFilePath = U"Voice/voice.wav";

	// 使用するスピーカー ID
	int32 speakerID; // スピーカー ID を指定

	// VOICEVOX 合成用 URL
	const URL singFrameAudioQueryURL = U"http://localhost:50021/sing_frame_audio_query?speaker=6000";
	URL frameSynthesisURL = U"http://localhost:50021/frame_synthesis?speaker={}"_fmt(speakerID);

	// VOICEVOX Speaker の取得
	Array<String> speakers;
	Array<int32> speakerIDs;
	ListBoxState SpeakerslistBoxState;
	for (const auto& speaker : VOICEVOX::GetSpeakers())
	{
		for (const auto& style : speaker.styles)
		{
			//if (speaker.name == U"ずんだもん") {
				speakers << U"{}（{}）"_fmt(speaker.name, style.name);
				speakerIDs << style.id;
			//}
		}
	}
	SpeakerslistBoxState = ListBoxState{ speakers };
	SpeakerslistBoxState.selectedItemIndex = 0;  // 初期選択
	s3d::Optional<uint64> previousSpeakersSelectedIndex;

	while (System::Update())
	{
		background1.draw(Arg::center = Scene::Center());
		characterTexture1.draw(Arg::center = Scene::Center()).scaled(0.5);

		//ClearConsole();

		if (SimpleGUI::Button(U"🎵 入力ファイルを選択", Vec2{ 1500, 800 }, unspecified))
		{
			inputpath = Dialog::OpenFile({ { U"VOICEVOX Project file", { U"vvproj" } } });
		}

		if (inputpath)
		{
			String inputfileName = FileSystem::BaseName(*inputpath);
			//Console << U"🎵 入力ファイル：" + inputfileName + U".vvproj";
		}

		if (SimpleGUI::Button(U"🎵 音声合成", Vec2{ 1500, 850 },unspecified, inputpath.has_value()))
		{
			String inputfileName = FileSystem::BaseName(*inputpath);
			FilePath savePath = U"Score/" + inputfileName + U".json";
			FilePath outputAudioFilePath = U"Voice/" + inputfileName + U"-" + speakers[SpeakerslistBoxState.selectedItemIndex.value()] + U".wav";

			VOICEVOX::ConvertVVProjToScoreJSON(*inputpath, savePath);
			//Console << U"✅ 変換成功： " + savePath;

			int32 speakerID = speakerIDs[SpeakerslistBoxState.selectedItemIndex.value()] + 3000;
			URL frameSynthesisURL = U"http://localhost:50021/frame_synthesis?speaker={}"_fmt(speakerID);

			if (VOICEVOX::SynthesizeFromJSONFileWrapperSplit(savePath, singQueryFilePath, outputAudioFilePath, singFrameAudioQueryURL, frameSynthesisURL, 3000))
			{
				audio1 = Audio{ Audio::Stream, outputAudioFilePath };	// 音声ファイルを読み込み
			}
		}

		if (SimpleGUI::Button(U"▶️再生", Vec2{1500, 900}, unspecified, !audio1.isEmpty())) {
			audio1.play();
		}

		SimpleGUI::ListBox(SpeakerslistBoxState, Vec2{ 400, 250 }, 300, 250);

		if (SpeakerslistBoxState.selectedItemIndex != previousSpeakersSelectedIndex)
		{
			previousSpeakersSelectedIndex = SpeakerslistBoxState.selectedItemIndex;
			String selectedSpeaker = speakers[SpeakerslistBoxState.selectedItemIndex.value()];
			FilePath base = U"Texture/Character/";
			FilePath texPath = base + selectedSpeaker + U".png";

			// ファイルが無ければデフォルトへ
			if (!FileSystem::Exists(texPath))
			{
				texPath = base + U"ずんだもん（ノーマル）.png";
			}
			characterTexture1 = Texture{ texPath };

		}
	}
}
