# include "Result.hpp"

Result::Result(const InitData& init)
	: IScene{ init }
{
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
		Console << U"⚠️ songAudio が空です。";
	}

	// 各フレームの画像と、次のフレームへのディレイ（ミリ秒）をロードする
	// 1️ ずんだもん
	if (gif.read(images, delays))
	{
		textures = images.map([](const Image& image) { return Texture{ image }; });
		images.clear();
	}
	else
	{
		Console << U"⚠️ gif1 (zunda_singing.gif) の読み込みに失敗しました";
	}
	// 2️ 晴れ
	if (gif2.read(images2, delays2))
	{
		textures2 = images2.map([](const Image& image2) { return Texture{ image2 }; });
		images2.clear();
	}
	else
	{
		Console << U"⚠️ gif2 (result_sunny.gif) の読み込みに失敗しました";
	}
}

void Result::update()
{

	if (ButtonAt(titleButtonCenter, titleButtonSize))
	{
		getData().instAudio.stop();
		getData().songAudio[0].stop();
		changeScene(U"Title", 0.3s);
	}
}

void Result::draw() const
{
	ClearPrint();

	double t = Scene::Time();

	// スコアに応じたアニメーション表示

		if (!textures2.isEmpty())
		{
			size_t frameIndex2 = AnimatedGIFReader::GetFrameIndex(t, delays2);
			textures2[frameIndex2].drawAt(Scene::Center());
		}

	// スコアに応じたキャラクター表示ではなく毎回固定表示
	if (!textures.isEmpty())
	{
		size_t frameIndex = AnimatedGIFReader::GetFrameIndex(t, delays);
		textures[frameIndex].drawAt(Scene::Center());
	}
	frame.drawAt(Scene::Center().movedBy(80, 0));
	titleButton.scaled(titleButtonScale).drawAt(titleButtonCenter);
	// --- スコアなど ---
	m_font(U"オリジナルおさかなソング")
		.drawAt(70, Scene::Center().movedBy(450, -250), goldColor);
	m_font(getData().songTitle+U"の曲で作った").drawAt(50, Scene::Center().movedBy(450, -420), goldColor);

	// --- 🎵 歌詞の色付き描画（字間調整つき） ---
	const ColorF normalColor = kogetyaColor;
	const ColorF userColor = kogetyaColor;
	const double fontSize = 50;
	const double letterSpacing = 0.0;

	Vec2 basePos = Scene::Center().movedBy(110, -70);
	Vec2 penPos = basePos;

	const Array<String> lines = getData().fullLyrics.removed(U'{').removed(U'}').split(U'\n');

	for (const auto& line : lines)
	{
		// 🔍 まずユーザー入力箇所の範囲を全て列挙
		Array<std::pair<size_t, size_t>> coloredRanges; // (開始, 終了)
		for (const auto& task : getData().solvedTasks)
		{
			size_t pos = 0;
			while ((pos = line.indexOf(task.userInput, pos)) != String::npos)
			{
				coloredRanges << std::make_pair(pos, pos + task.userInput.size());
				pos += task.userInput.size();
			}
		}

		// --- 実際の描画 ---
		for (size_t i = 0; i < line.size(); ++i)
		{
			bool isColored = false;
			for (const auto& range : coloredRanges)
			{
				if (i >= range.first && i < range.second)
				{
					isColored = true;
					break;
				}
			}

			const String s(1, line[i]);
			const ColorF color = isColored ? userColor : normalColor;
			m_font(s).draw(fontSize, penPos, color);
			penPos.x += m_font(s).region().w + letterSpacing;
		}

		// 改行処理
		penPos.x = basePos.x;
		penPos.y += m_font.height() * 1.3;
	}
}

