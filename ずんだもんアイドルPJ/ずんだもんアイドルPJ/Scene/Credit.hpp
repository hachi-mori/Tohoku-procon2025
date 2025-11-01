# pragma once
# include "../Common.hpp"

// ゲームシーン
class Credit : public App::Scene
{
public:

	Credit(const InitData& init);

	void update() override;

	void draw() const override;

private:
	Texture background{ Resource(U"Texture/assets/credit.png") };
};
