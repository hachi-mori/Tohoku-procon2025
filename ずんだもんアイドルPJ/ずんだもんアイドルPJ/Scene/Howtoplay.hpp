# pragma once
# include "../Common.hpp"

// ゲームシーン
class Howtoplay : public App::Scene
{
public:

	Howtoplay(const InitData& init);

	void update() override;

	void draw() const override;

private:
	Texture background{ Resource(U"Texture/assets/howtoplay.png") };
};
