# include "Common.hpp"
# include "Scene/Scene1.hpp"
# include "Scene/Scene2.hpp"

void Main()
{
	Window::SetTitle(U"マーブルレース");
	Window::Resize(1920, 1080);
	Scene::SetResizeMode(ResizeMode::Keep);
	Window::SetStyle(WindowStyle::Sizable);

	App manager;
	manager.add<Scene1>(U"Scene1");
	manager.add<Scene2>(U"Scene2");

	while (System::Update())
	{
		// 🔴 デバッグ用：数字キーでシーンを切り替え
		if (Key1.down() || KeyNum1.down()) { manager.changeScene(U"Scene1"); }
		if (Key2.down() || KeyNum2.down()) { manager.changeScene(U"Scene2"); }

		if (not manager.update())
		{
			break;
		}
	}
}
