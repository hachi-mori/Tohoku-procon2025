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

	// ボタンの位置
	Vec2 startButtonPos{ Scene::Center()};
};
