# include "../Common.hpp"

//------------------------------------------------------
// Scene3 : ゲームシーン
//------------------------------------------------------
class Scene3 : public App::Scene
{
public:
	Scene3(const InitData& init);

	void update() override;
	void draw() const override;

private:
	Array<String> vvprojNames;
	mutable ListBoxState listBoxStateVV;
	Optional<FilePath> selectedVVProjPath;
	void initVVProjList();
	Texture background{ U"Texture/assets/title_background.png" };
	Texture logo{ U"Texture/assets/title_logo.png" };
	Texture frame{ U"Texture/assets/title_frame_w_trans.png" };

	Texture startButton{ U"Texture/assets/button/start.png" };
	double startButtonScale = 0.7;
	Vec2 startButtonCenter = Scene::Center().movedBy(0, 400);
	SizeF startButtonSize = startButton.size() * (startButtonScale - 0.05);	// 画像スケールから少しだけ小さくする

	Texture storyButton{ U"Texture/assets/button/story.png" };
	double storyButtonScale = 0.7;
	Vec2 storyButtonCenter = Scene::Center().movedBy(-450, 400);
	SizeF storyButtonSize = storyButton.size() * (storyButtonScale - 0.05);  // 画像スケールから少しだけ小さくする

	Texture howtoplayButton{ U"Texture/assets/button/howtoplay.png" };
	double howtoplayButtonScale = 0.7;
	Vec2 howtoplayButtonCenter = Scene::Center().movedBy(450, 400);
	SizeF howtoplayButtonSize = howtoplayButton.size() * (howtoplayButtonScale - 0.05);  // 画像スケールから少しだけ小さくする
};
