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
	if (ButtonAt(titleButtonCenter, titleButtonSize))
	{
		changeScene(U"Scene3", 0.3s);
	}
}

void Result::draw() const
{
	ClearPrint();

	double t = Scene::Time();
	double t2 = Scene::Time();

	// 背景アニメーション
	if (!textures.isEmpty())
	{
		size_t frameIndex = AnimatedGIFReader::GetFrameIndex(t, delays);
		textures[frameIndex].drawAt(Scene::Center());
	}
	if (!textures2.isEmpty())
	{
		size_t frameIndex2 = AnimatedGIFReader::GetFrameIndex(t2, delays2);
		textures2[frameIndex2].drawAt(Scene::Center());
	}

	frame.drawAt(Scene::Center().movedBy(80, 0));
	titleButton.scaled(titleButtonScale).drawAt(titleButtonCenter);

	// --- スコアなど ---
	m_font(ToString(static_cast<double>(getData().finalRhymeMatchPercent), 0) + U" %")
		.drawAt(150, Scene::Center().movedBy(540, -250), goldColor);
	m_font(U"あなたのスコア").drawAt(35, Scene::Center().movedBy(150, -300), goldColor);
	m_font(U"🎵" + getData().songTitle).drawAt(40, Scene::Center().movedBy(450, -450), goldColor);

	// --- 🎵 歌詞の色付き描画（字間調整つき） ---
	const ColorF normalColor = kogetyaColor;
	const ColorF userColor = Palette::Orange;
	const double fontSize = 62;
	const double letterSpacing = 10.0;

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

