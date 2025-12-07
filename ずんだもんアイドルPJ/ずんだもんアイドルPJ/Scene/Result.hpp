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
	Texture frame{ Resource(U"Texture/assets/result_frame.png") };
	Texture kakusei{ Resource(U"Texture/assets/zunda_kakusei.png") };
	Texture sippai{ Resource(U"Texture/assets/zunda_sippai.png") };

	// GIF アニメーション画像を開く
	const AnimatedGIFReader gif{ Resource(U"Texture/assets/zunda_singing.gif") };
	Array<Image> images;
	mutable Array<int32> delays;
	Array<Texture> textures;
	// 2つ目のGIFアニメーション画像を開く
	const AnimatedGIFReader gif2{ Resource(U"Texture/assets/result_sunny.gif") };
	Array<Image> images2;
	mutable Array<int32> delays2;
	Array<Texture> textures2;
	// 3つ目のGIFアニメーション画像を開く
	const AnimatedGIFReader gif3{ Resource(U"Texture/assets/result_rainy.gif") };
	Array<Image> images3;
	mutable Array<int32> delays3;
	Array<Texture> textures3;
	// 4つ目のGIFアニメーション画像を開く
	const AnimatedGIFReader gif4{ Resource(U"Texture/assets/result_cloudy.gif") };
	Array<Image> images4;
	mutable Array<int32> delays4;
	Array<Texture> textures4;

	const FilePath fontpath = Resource(U"Texture/Futehodo-MaruGothic.ttf");
	Font m_font{ FontMethod::MSDF, 60 , fontpath };
	Color kogetyaColor = { 134,79,9 };
	Color goldColor = { 243,174,6 };

	Texture titleButton{ Resource(U"Texture/assets/button/title.png") };
	double titleButtonScale = 1.0;
	Vec2 titleButtonCenter = Scene::Center().movedBy(-850, 440);
	SizeF titleButtonSize = titleButton.size() * (titleButtonScale - 0.05);	// 画像スケールから少しだけ小さくする
};
