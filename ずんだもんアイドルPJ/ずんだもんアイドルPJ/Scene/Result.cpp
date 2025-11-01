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

	// 各フレームの画像と、次のフレームへのディレイ（ミリ秒）をロードする
	// 1️ ずんだもん
	if (gif.read(images, delays))
	{
		textures = images.map([](const Image& image) { return Texture{ image }; });
		images.clear();
	}
	else
	{
		Print << U"⚠️ gif1 (zunda_singing.gif) の読み込みに失敗しました";
	}
	// 2️ 晴れ
	if (gif2.read(images2, delays2))
	{
		textures2 = images2.map([](const Image& image2) { return Texture{ image2 }; });
		images2.clear();
	}
	else
	{
		Print << U"⚠️ gif2 (result_sunny.gif) の読み込みに失敗しました";
	}
	// 3️ 雨
	if (gif3.read(images3, delays3))
	{
		textures3 = images3.map([](const Image& image3) { return Texture{ image3 }; });
		images3.clear();
	}
	else
	{
		Print << U"⚠️ gif3 (result_rainy.gif) の読み込みに失敗しました";
	}
	// 4️ 曇り
	if (gif4.read(images4, delays4))
	{
		textures4 = images4.map([](const Image& image4) { return Texture{ image4 }; });
		images4.clear();
	}
	else
	{
		Print << U"⚠️ gif4 (result_cloudy.gif) の読み込みに失敗しました";
	}
}

void Result::PostResultTweet(int32 score)
{
	const String text = U"ぼくは {} 点をとったのだ！\n#Siv3D\n#シングリンク\n#ずんだもん\nhttps://scrapbox.io/bnscup2025/%E3%82%B7%E3%83%B3%E3%82%B0%E3%83%AA%E3%83%B3%E3%82%AF"_fmt(score);

	Twitter::OpenTweetWindow(text);
}

void Result::update()
{
	if (SimpleGUI::Button(U"Share Result", Vec2{ 1800, 970 }))
	{
		PostResultTweet(static_cast<int>(getData().finalRhymeMatchPercent));
	}

	if (ButtonAt(titleButtonCenter, titleButtonSize))
	{
		changeScene(U"Scene3", 0.3s);
	}

	if (!getData().instAudio.isPlaying()) {
		if (ButtonAt(restartButtonCenter, restartButtonSize)) {
			getData().instAudio.setVolume(0.4);
			getData().songAudio[0].play();
			getData().instAudio.play();
		}
	}
}

void Result::draw() const
{
	ClearPrint();

	double t = Scene::Time();

	// スコアに応じたアニメーション表示
	if (50 < getData().finalRhymeMatchPercent) {
		if (!textures2.isEmpty())
		{
			size_t frameIndex2 = AnimatedGIFReader::GetFrameIndex(t, delays2);
			textures2[frameIndex2].drawAt(Scene::Center());
		}
	}
	else if (25 < getData().finalRhymeMatchPercent && getData().finalRhymeMatchPercent <= 50) {
		if (!textures3.isEmpty())
		{
			size_t frameIndex3 = AnimatedGIFReader::GetFrameIndex(t, delays3);
			textures3[frameIndex3].drawAt(Scene::Center());
		}
	}else if (getData().finalRhymeMatchPercent <= 25) {
		if (!textures4.isEmpty())
		{
			size_t frameIndex4 = AnimatedGIFReader::GetFrameIndex(t, delays4);
			textures4[frameIndex4].drawAt(Scene::Center());
		}
	}

	// スコアに応じたキャラクター表示
	if (90 < getData().finalRhymeMatchPercent) {
		if (!kakusei.isEmpty())
		{
			kakusei.scaled(0.9).drawAt(Scene::Center().movedBy(-400, -60));
		}
	}else if ((10 < getData().finalRhymeMatchPercent && getData().finalRhymeMatchPercent <= 90)) {
		if (!textures.isEmpty())
		{
			size_t frameIndex = AnimatedGIFReader::GetFrameIndex(t, delays);
			textures[frameIndex].drawAt(Scene::Center());
		}
	}else if (getData().finalRhymeMatchPercent <= 10) {
		if (!sippai.isEmpty())
		{
			sippai.draw();
		}
	}
	
	frame.drawAt(Scene::Center().movedBy(80, 0));
	titleButton.scaled(titleButtonScale).drawAt(titleButtonCenter);
	if (!getData().instAudio.isPlaying()){restartButton.scaled(restartButtonScale).drawAt(restartButtonCenter);}

	// --- スコアなど ---
	m_font(ToString(static_cast<double>(getData().finalRhymeMatchPercent), 0) + U" %")
		.drawAt(150, Scene::Center().movedBy(540, -250), goldColor);
	m_font(U"あなたのスコア").drawAt(35, Scene::Center().movedBy(150, -300), goldColor);
	m_font(getData().songTitle).drawAt(50, Scene::Center().movedBy(450, -420), goldColor);

	// --- 🎵 歌詞の色付き描画（字間調整つき） ---
	const ColorF normalColor = kogetyaColor;
	const ColorF userColor = Palette::Orange;
	const double fontSize = 52;
	const double letterSpacing = 0.5;

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
	SimpleGUI::Button(U"Share Result", Vec2{ 1700, 970 });
}

