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
	if (SimpleGUI::ButtonAt(U"スタート", Vec2{ Scene::Center().x, Scene::Center().y + 400 }, 100))
	{
		getData().vvprojPath = *selectedVVProjPath;
		//Print << U"選択された vvproj: " << *selectedVVProjPath;
		// シーン切り替え
		changeScene(U"Scene2",0.3s);
	}
}

void Scene3::draw() const
{
	background.draw();
	logo.scaled(0.75).drawAt(Scene::Center().x,Scene::Center().y-60);

	SimpleGUI::ListBoxAt(listBoxStateVV, Vec2{ Scene::Center().x, Scene::Center().y + 200}, 400, 200);
	SimpleGUI::ButtonAt(U"スタート", Vec2{ Scene::Center().x, Scene::Center().y + 400 }, 100);
}
