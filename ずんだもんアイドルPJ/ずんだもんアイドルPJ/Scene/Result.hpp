# pragma once
# include "../Common.hpp"

// ゲームシーン
class Result : public App::Scene
{
public:

	Result(const InitData& init);

	void update() override;

	void draw() const override;

private:
	Texture frame{ U"Texture/assets/result_frame.png" };

	// GIF アニメーション画像を開く
	const AnimatedGIFReader gif{ U"Texture/assets/result_background.gif" };
	Array<Image> images;
	mutable Array<int32> delays;
	Array<Texture> textures;
	// 2つ目のGIFアニメーション画像を開く
	const AnimatedGIFReader gif2{ U"Texture/assets/zunda_singing.gif" };
	Array<Image> images2;
	mutable Array<int32> delays2;
	Array<Texture> textures2;

	const FilePath fontpath = U"Texture/Futehodo-MaruGothic.ttf";
	Font m_font{ FontMethod::MSDF, 60 , fontpath };
	Color kogetyaColor = { 134,79,9 };
	Color goldColor = { 243,174,6 };

	Texture titleButton{ U"Texture/assets/button/title.png" };
	double titleButtonScale = 1.0;
	Vec2 titleButtonCenter = Scene::Center().movedBy(-850, 430);
	SizeF titleButtonSize = titleButton.size() * (titleButtonScale - 0.05);	// 画像スケールから少しだけ小さくする
};
