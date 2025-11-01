# include "Result.hpp"

Result::Result(const InitData& init)
	: IScene{ init }
{
	//Print << U"===== Result Scene =====";
	// --- Scene1 からデータを受け取る ---

	// --- シーン開始時に音声を自動再生 ---
	getData().instAudio.setVolume(0.4);
	getData().instAudio.play();

	// キャラが1人だと仮定して、その音声を再生
	if (!getData().songAudio.isEmpty())
	{
		getData().songAudio[0].play();
	}
	else
	{
		Print << U"⚠️ songAudio が空です。";
	}

	// GIFに関する処理
	// 各フレームの画像と、次のフレームへのディレイ（ミリ秒）をロードする
	// GIFに関する処理
	if (gif.read(images, delays)) // 👈 読み込みが成功したら
	{
		textures = images.map([](const Image& image) { return Texture{ image }; });
		images.clear();
	}
	else
	{
		Print << U"⚠️ zunda_singing.gif の読み込みに失敗しました！"; // 失敗時のログ
	}

	if (gif2.read(images2, delays2)) // 👈 読み込みが成功したら
	{
		textures2 = images2.map([](const Image& image2) { return Texture{ image2 }; });
		images2.clear();
	}
	else
	{
		Print << U"⚠️ result_background.gif の読み込みに失敗しました！"; // 失敗時のログ
	}
}

void Result::update()
{
}

void Result::draw() const
{
	//GIFアニメーションの描画
	ClearPrint();

	// アニメーションの経過時間
	double t = Scene::Time();
	double t2 = Scene::Time();

	// 経過時間と各フレームのディレイに基づいて、何番目のフレームを描けばよいかを計算する
	size_t frameIndex = AnimatedGIFReader::GetFrameIndex(t, delays);
	size_t frameIndex2 = AnimatedGIFReader::GetFrameIndex(t2, delays2);

	textures[frameIndex].drawAt(Scene::Center());

	if (!textures.isEmpty()) // 👈 texturesが空でなければ描画
	{
		size_t frameIndex = AnimatedGIFReader::GetFrameIndex(t, delays);
		textures[frameIndex].drawAt(Scene::Center());
	}
	if (!textures2.isEmpty()) // 👈 textures2が空でなければ描画
	{
		size_t frameIndex2 = AnimatedGIFReader::GetFrameIndex(t2, delays2);
		textures2[frameIndex2].drawAt(Scene::Center());
	}
}
