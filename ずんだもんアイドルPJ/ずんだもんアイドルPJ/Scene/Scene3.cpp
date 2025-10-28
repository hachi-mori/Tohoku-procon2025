#include "Scene3.hpp"

Scene3::Scene3(const InitData& init)
	: IScene{ init }
{
	initVVProjList();
}

void Scene3::initVVProjList()
{
	const FilePath scoreDir = U"Score";

	for (const auto& path : FileSystem::DirectoryContents(scoreDir, Recursive::No))
	{
		if (FileSystem::Extension(path) == U"vvproj")
		{
			vvprojNames << FileSystem::RelativePath(path);
		}
	}

	listBoxStateVV = ListBoxState{ vvprojNames };
}

void Scene3::update()
{
	if (listBoxStateVV.selectedItemIndex)
	{
		selectedVVProjPath = vvprojNames[*listBoxStateVV.selectedItemIndex];
	}
	// 画面上にボタンを出して、
	   // そのボタンが押されたフレームで true が返る
	if (SimpleGUI::Button(U"スタート", startButtonPos))
	{
		getData().vvprojPath = *selectedVVProjPath;
		Print << U"選択された vvproj: " << *selectedVVProjPath;
		// シーン切り替え
		changeScene(U"Scene2",0.3s);
	}
}

void Scene3::draw() const
{
	SimpleGUI::ListBox(listBoxStateVV, Vec2{ 400, 250 }, 300, 400);

	if (selectedVVProjPath)
	{
		FontAsset(U"MainFont")(U"選択中: {}"_fmt(*selectedVVProjPath))
			.draw(30, Vec2{ 400, 700 }, Palette::White);
	}

}
