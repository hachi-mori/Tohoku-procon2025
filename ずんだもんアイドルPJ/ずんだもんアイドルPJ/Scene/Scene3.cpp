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

	if (ButtonAt(startButtonCenter, startButtonSize))
	{
		if (selectedVVProjPath)  // 中身があるか確認！
		{
			getData().vvprojPath = *selectedVVProjPath;
			changeScene(U"Scene2", 0.3s);
		}
		else
		{
			Print << U"vvprojが選択されていません。";
		}
	}
}

void Scene3::draw() const
{
	background.draw();
	logo.scaled(0.85).drawAt(Scene::Center().x,Scene::Center().y-70);
	
	SimpleGUI::ListBoxAt(listBoxStateVV, Vec2{ Scene::Center().x+2, Scene::Center().y + 199}, 440, 238);

	frame.scaled(1.1).drawAt(Scene::Center().x, Scene::Center().y + 70);
	startButton.scaled(startButtonScale).drawAt(startButtonCenter);
}
