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
			changeScene(U"Scene2", 0.3s);
		}
		else
		{
			Print << U"vvprojが選択されていません。";
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
	
	SimpleGUI::ListBoxAt(listBoxStateVV, Vec2{ Scene::Center().x+2, Scene::Center().y + 209}, 440, 238);

	frame.scaled(1.1).drawAt(Scene::Center().x, Scene::Center().y + 80);

	startButton.scaled(startButtonScale).drawAt(startButtonCenter);
	storyButton.scaled(storyButtonScale).drawAt(storyButtonCenter);
	howtoplayButton.scaled(howtoplayButtonScale).drawAt(howtoplayButtonCenter);
	creditButton.scaled(creditButtonScale).drawAt(creditButtonCenter);
}
