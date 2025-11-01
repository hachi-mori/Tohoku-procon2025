# include "Common.hpp"
# include "Scene/Scene1.hpp"
# include "Scene/Scene2.hpp"
# include "Scene/Scene3.hpp"
# include "Scene/Result.hpp"

void Main()
{
	Window::SetTitle(U"シングリンク");
	Window::Resize(1920, 1080);
	Scene::SetResizeMode(ResizeMode::Keep);
	Window::SetStyle(WindowStyle::Sizable);

	App manager;

	manager.add<Scene3>(U"Scene3"); // タイトル
	manager.add<Scene2>(U"Scene2"); // 歌詞入力
	manager.add<Scene1>(U"Scene1"); // ライブシーン
	manager.add<Result>(U"Result"); // ライブシーン

	while (System::Update())
	{
		// 🔴 デバッグ用：数字キーでシーンを切り替え
		if (Key1.down() || KeyNum1.down()) { manager.changeScene(U"Scene1"); }
		if (Key2.down() || KeyNum2.down()) { manager.changeScene(U"Scene2"); }
		if (Key3.down() || KeyNum3.down()) { manager.changeScene(U"Scene3"); }
		if (Key4.down() || KeyNum4.down()) { manager.changeScene(U"Result"); }

		if (not manager.update())
		{
			break;
		}
	}
}
