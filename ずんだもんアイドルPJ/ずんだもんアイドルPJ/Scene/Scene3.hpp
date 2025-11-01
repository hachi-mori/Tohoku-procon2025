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
	SizeF startButtonSize = startButton.size() * startButtonScale;  // ✅ 画像スケールからサイズ取得
};
