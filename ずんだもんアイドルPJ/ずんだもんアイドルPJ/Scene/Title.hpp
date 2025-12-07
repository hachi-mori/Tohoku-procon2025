# pragma once
# include "../Common.hpp"

//------------------------------------------------------
// Title : タイトル
//------------------------------------------------------
class Title : public App::Scene
{
public:
	Title(const InitData& init);

	void update() override;
	void draw() const override;

private:

	Array<String> vvprojNames;
	mutable ListBoxState listBoxStateVV;
	Optional<FilePath> selectedVVProjPath;
	void initVVProjList();
	Texture background{ Resource(U"Texture/assets/title_background.png") };
	Texture logo{ Resource(U"Texture/assets/title_logo.png") };
	Texture frame{ Resource(U"Texture/assets/title_frame_w_trans.png") };

	Texture startButton{ Resource(U"Texture/assets/button/start.png") };
	double startButtonScale = 0.7;
	Vec2 startButtonCenter = Scene::Center().movedBy(0, 430);
	SizeF startButtonSize = startButton.size() * (startButtonScale - 0.05);	// 画像スケールから少しだけ小さくする

	Texture storyButton{ Resource(U"Texture/assets/button/story.png") };
	double storyButtonScale = 0.7;
	Vec2 storyButtonCenter = Scene::Center().movedBy(-450, 430);
	SizeF storyButtonSize = storyButton.size() * (storyButtonScale - 0.05);  // 画像スケールから少しだけ小さくする

	Texture howtoplayButton{ Resource(U"Texture/assets/button/howtoplay.png") };
	double howtoplayButtonScale = 0.7;
	Vec2 howtoplayButtonCenter = Scene::Center().movedBy(450, 430);
	SizeF howtoplayButtonSize = howtoplayButton.size() * (howtoplayButtonScale - 0.05);  // 画像スケールから少しだけ小さくする

	Texture creditButton{ Resource(U"Texture/assets/button/credit.png") };
	double creditButtonScale = 0.7;
	Vec2 creditButtonCenter = Scene::Center().movedBy(770, -450);
	SizeF creditButtonSize = creditButton.size() * (creditButtonScale - 0.05);  // 画像スケールから少しだけ小さくする

	mutable bool gameStartFlag = false;
	mutable bool selectVVProjFlag = false;

	const FilePath fontpath = Resource(U"Texture/Futehodo-MaruGothic.ttf");
	Font m_font{ FontMethod::MSDF, 40 , fontpath };
	Color kogetyaColor = { 134,79,9 };

	mutable TextEditState urlBox;
	String okVersion = U"0.25.0";
	mutable String connectToVoiceVoxText;
	void checkVVVersion();

	mutable bool urlBoxPrevious = false;
};
