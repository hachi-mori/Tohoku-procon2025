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
	// === Scene1 から引き継ぐデータ ===
	Array<Audio> songAudio;
	Audio audio_inst;

	FilePath vvprojPath;
	String songTitle;

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
};
