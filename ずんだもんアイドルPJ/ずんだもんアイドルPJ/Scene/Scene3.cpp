#include "Scene3.hpp"

Scene3::Scene3(const InitData& init)
	: IScene{ init }
{
	initVVProjList();

	if (not getData().voicevoxCheckedFlag) {
		const String version = VOICEVOX::GetEngineVersion(baseURL, 2s);
		// Print << U"🎤 VOICEVOX Engine Version: " << version;
		// VOICEVOX の接続状態とバージョンを確認
		if (version == U"(接続エラー)")
		{
			const String msg = U"ゲーム「シングリンク」のすべての機能を利用するには、"
				U"アプリ「VOICEVOX（無料）」をインストールし、起動した状態でゲームを起動する必要があります。\n\n"
				U"VOICEVOXを使用しない簡易版ゲーム「シングリンク（簡易モード）」で遊びますか？\n"
				U"※ 簡易モードではゲーム体験が著しく制限されます。";

			if (System::MessageBoxYesNo(msg) == MessageBoxResult::No)
			{
				if (System::MessageBoxYesNo(U"最新の「VOICEVOX（無料）」をインストールしますか？（公式サイトに移動します）") == MessageBoxResult::Yes)
				{
					System::LaunchBrowser(U"https://voicevox.hiroshiba.jp/");
				}
				System::Exit();
			}
		}
		else if (version != U"0.25.0")
		{
			const String msg = U"VOICEVOXのバージョンが動作保証バージョンよりも古いため、"
				U"正常に動作しない可能性があります。\n\n"
				U"動作保証バージョン：0.25.0 以降\n"
				U"現在のバージョン：" + version + U"\n\n"
				U"このままゲームを起動しますか？";

			if (System::MessageBoxYesNo(msg) == MessageBoxResult::No)
			{
				if (System::MessageBoxYesNo(U"最新の「VOICEVOX（無料）」をインストールしますか？（公式サイトに移動します）") == MessageBoxResult::Yes)
				{
					System::LaunchBrowser(U"https://voicevox.hiroshiba.jp/");
				}
				System::Exit();
			}
		}
		getData().voicevoxCheckedFlag = true;
	}

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

	logo.scaled(0.81).drawAt(Scene::Center().x,Scene::Center().y-50);
	
	SimpleGUI::ListBoxAt(listBoxStateVV, Vec2{ Scene::Center().x+2, Scene::Center().y + 208}, 440, 238);

	frame.scaled(1.1).drawAt(Scene::Center().x, Scene::Center().y + 80);

	startButton.scaled(startButtonScale).drawAt(startButtonCenter);
	storyButton.scaled(storyButtonScale).drawAt(storyButtonCenter);
	howtoplayButton.scaled(howtoplayButtonScale).drawAt(howtoplayButtonCenter);
	creditButton.scaled(creditButtonScale).drawAt(creditButtonCenter);

	if (selectVVProjFlag) {
		m_font(U"←きょくをえらんでね！").drawAt(40, Scene::Center().movedBy(460, 140), kogetyaColor);
	}
}
