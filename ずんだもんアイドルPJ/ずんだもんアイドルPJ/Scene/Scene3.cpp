#include "Scene3.hpp"

Scene3::Scene3(const InitData& init)
	: IScene{ init }
{
	initVVProjList();
	if (not getData().voicevoxCheckedFlag) urlBox.text = U"http://localhost:50021";
	checkVVVersion();

	// --- モニタ情報を取得 ---
	const Array<MonitorInfo> monitors = System::EnumerateMonitors();
	const size_t currentMonitorIndex = System::GetCurrentMonitorIndex();

	if (monitors.isEmpty() || currentMonitorIndex >= monitors.size())
	{
		Print << U"[Error] モニタ情報の取得に失敗しました。";
		System::Exit();  // または安全なデフォルト動作
	}

	const auto& monitor = monitors[currentMonitorIndex];

	// ✅ 正しいモニタの表示領域サイズを使用
	const Size monitorSize = monitor.displayRect.size;
	const Size targetSize{ 1920, 1080 };
	// 想定よりモニタが小さい場合のみフルスクリーン化
	if (monitorSize.x <= targetSize.x || monitorSize.y <= targetSize.y)
	{
		Window::SetFullscreen(true, currentMonitorIndex);
	}

	//Print << U"モニタ名: " << monitor.name;
	//Print << U"モニタサイズ: " << monitorSize;
	//Print << U"フルスクリーン: " << Window::GetState().fullscreen;
}

void Scene3::initVVProjList()
{
	const FilePath scoreDir = U"Score";

	for (const auto& path : FileSystem::DirectoryContents(scoreDir, Recursive::No))
	{
		if (FileSystem::Extension(path) == U"vvproj")
		{
			vvprojNames << FileSystem::BaseName(path);
		}
	}

	listBoxStateVV = ListBoxState{ vvprojNames };
}

void Scene3::checkVVVersion() {
	getData().baseURL = urlBox.text;
	const String version = VOICEVOX::GetEngineVersion(getData().baseURL, 1s);

	if (version == U"(接続エラー)")
	{
		connectToVoiceVoxText = U"VOICEVOX：接続できていないのだ...";
	}
	else if (version < okVersion)
	{
		connectToVoiceVoxText = U"VOICEVOX：バージョンが古いかもなのだ～";
	}
	else {
		connectToVoiceVoxText = U"VOICEVOX：OKなのだ！";
	}
}

void Scene3::update()
{
	if (listBoxStateVV.selectedItemIndex)
	{
		selectedVVProjPath = U"Score/" + vvprojNames[*listBoxStateVV.selectedItemIndex] + U".vvproj";
	}

	if (ButtonAt(startButtonCenter, startButtonSize))
	{
		if (selectedVVProjPath)  // 中身があるか確認！
		{
			getData().vvprojPath = *selectedVVProjPath;
			getData().songTitle = FileSystem::BaseName(*selectedVVProjPath);
			getData().baseURL = urlBox.text;

			getData().voicevoxCheckedFlag = true;
			const String version = VOICEVOX::GetEngineVersion(getData().baseURL, 1s);
			// Print << U"🎤 VOICEVOX Engine Version: " << version;
			// VOICEVOX の接続状態とバージョンを確認
			if (version == U"(接続エラー)")
			{
				const String msg = U"ゲーム「シングリンク」のすべての機能を利用するには、"
					U"アプリ「VOICEVOX（無料）」をインストールし、起動した状態でゲームを開始する必要があります。\n\n"
					U"VOICEVOXを使用しない簡易版ゲーム「シングリンク（簡易モード）」で遊びますか？\n"
					U"※ 簡易モードではゲーム体験が著しく制限されます。";

				if (System::MessageBoxYesNo(msg) == MessageBoxResult::No)
				{
					if (System::MessageBoxYesNo(U"最新の「VOICEVOX（無料）」をインストールしますか？（公式サイトに移動します）") == MessageBoxResult::Yes)
					{
						System::LaunchBrowser(U"https://voicevox.hiroshiba.jp/");
					}
					return;
				}
			}
			else if (version < okVersion)
			{
				const String msg = U"VOICEVOXのバージョンが動作保証バージョンよりも古いため、"
					U"正常に動作しない可能性があります。\n\n"
					U"動作保証バージョン：" + okVersion + U" 以降\n"
					U"現在のバージョン：" + version + U"\n\n"
					U"このままゲームを開始しますか？";

				if (System::MessageBoxYesNo(msg) == MessageBoxResult::No)
				{
					if (System::MessageBoxYesNo(U"最新の「VOICEVOX（無料）」をインストールしますか？（公式サイトに移動します）") == MessageBoxResult::Yes)
					{
						System::LaunchBrowser(U"https://voicevox.hiroshiba.jp/");
					}
					return;
				}
			}

			changeScene(U"Scene2", 0.3s);
		}
		else
		{
			selectVVProjFlag = true;
		}
	}
	if (ButtonAt(storyButtonCenter, storyButtonSize))
	{
		changeScene(U"Story", 0.3s);
	}
	if (ButtonAt(howtoplayButtonCenter, howtoplayButtonSize))
	{
		changeScene(U"Howtoplay", 0.3s);
	}
	if (ButtonAt(creditButtonCenter, creditButtonSize))
	{
		changeScene(U"Credit", 0.3s);
	}

}

void Scene3::draw() const
{
	background.draw();

	logo.scaled(0.81).drawAt(Scene::Center().x, Scene::Center().y - 50);

	SimpleGUI::ListBoxAt(listBoxStateVV, Vec2{ Scene::Center().x + 2, Scene::Center().y + 208 }, 440, 238);

	frame.scaled(1.1).drawAt(Scene::Center().x, Scene::Center().y + 80);

	startButton.scaled(startButtonScale).drawAt(startButtonCenter);
	storyButton.scaled(storyButtonScale).drawAt(storyButtonCenter);
	howtoplayButton.scaled(howtoplayButtonScale).drawAt(howtoplayButtonCenter);
	creditButton.scaled(creditButtonScale).drawAt(creditButtonCenter);

	if (selectVVProjFlag) {
		m_font(U"←きょくをえらんでね！").drawAt(40, Scene::Center().movedBy(460, 140), kogetyaColor);
	}

	m_font(connectToVoiceVoxText).draw(20, Vec2{20,50}, kogetyaColor);
	
	// フォーカス状態の前回値を取得（TextBox 描画の前に！）
	urlBoxPrevious = urlBox.active;

	// テキストボックス描画（ここで active が更新される）
	SimpleGUI::TextBoxAt(urlBox, Vec2{ 160, 110 }, 300);

	// ↓ここで状態の変化を検出
	if (urlBoxPrevious && (urlBox.active == false))
	{
		getData().baseURL = urlBox.text;
		const String version = VOICEVOX::GetEngineVersion(getData().baseURL, 1s);

		if (version == U"(接続エラー)")
		{
			connectToVoiceVoxText = U"VOICEVOX：接続できていないのだ...";
		}
		else if (version < okVersion)
		{
			connectToVoiceVoxText = U"VOICEVOX：バージョンが古いかもなのだ～";
		}
		else {
			connectToVoiceVoxText = U"VOICEVOX：OKなのだ！";
		}
	}
}
