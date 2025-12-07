# include "Common.hpp"
# include "Scene/VocalSynthesis.hpp"
# include "Scene/WriteLyrics.hpp"
# include "Scene/Title.hpp"
# include "Scene/Result.hpp"
# include "Scene/Story.hpp"
# include "Scene/Howtoplay.hpp"
# include "Scene/Credit.hpp"

void Main()
{
	// タイトルとウィンドウ設定
	Window::SetTitle(U"おさかな呼び込みずんだもん");
	const Size targetSize{ 1920, 1080 };
	Window::Resize(targetSize);
	Scene::SetResizeMode(ResizeMode::Keep);
	Window::SetStyle(WindowStyle::Sizable);

	App manager;

	manager.add<Title>(U"Title"); // タイトル
	manager.add<WriteLyrics>(U"WriteLyrics"); // 歌詞入力
	manager.add<VocalSynthesis>(U"VocalSynthesis"); // 歌声生成
	manager.add<Result>(U"Result"); // 歌声再生
	manager.add<Story>(U"Story"); // ストーリー
	manager.add<Howtoplay>(U"Howtoplay"); // あそびかた
	manager.add<Credit>(U"Credit"); // クレジット

	while (System::Update())
	{
		// 🔴 デバッグ用：数字キーでシーンを切り替え
		if (Key1.down() || KeyNum1.down()) { manager.changeScene(U"VocalSynthesis"); }
		if (Key2.down() || KeyNum2.down()) { manager.changeScene(U"WriteLyrics"); }
		if (Key3.down() || KeyNum3.down()) { manager.changeScene(U"Title"); }
		if (Key4.down() || KeyNum4.down()) { manager.changeScene(U"Result"); }
		if (Key5.down() || KeyNum5.down()) { manager.changeScene(U"Story"); }
		if (Key6.down() || KeyNum6.down()) { manager.changeScene(U"Howtoplay"); }
		if (Key7.down() || KeyNum7.down()) { manager.changeScene(U"Credit"); }
		
		if (not manager.update())
		{
			break;
		}
	}
}
