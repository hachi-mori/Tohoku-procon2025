# pragma once
# include "../Common.hpp"

// ゲームシーン
class Story : public App::Scene
{
public:

	Story(const InitData& init);

	void update() override;

	void draw() const override;

private:
	Texture background{ Resource(U"Texture/assets/story.png") };
};
