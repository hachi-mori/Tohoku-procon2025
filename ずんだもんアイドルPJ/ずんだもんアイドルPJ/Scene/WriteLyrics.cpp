#include "WriteLyrics.hpp"

// WriteLyrics::WriteLyrics (コンストラクタ) 
WriteLyrics::WriteLyrics(const InitData& init)
	: IScene{ init }, m_textState{}
{
	m_textState.active = true;

	talkLines = VOICEVOX::ExtractTalkUtterances(getData().vvprojPath);

	// ===============================
	// 🎯 ここで 1〜5問目のお題を決める
	//    - fontSize: フォントサイズ
	//    - text    : 画面中央に表示するお題テキスト
	// ===============================
	m_topics = {
		{ 100, U"① やすいおさかなは？" },
		{ 90, U"② おいしいおさかなは？" },
		{ 90, U"③ しゅんのおさかなは？" },
		{ 100, U"④ すきなおさかなは？" },
		{ 80, U"⑤ いまたべたいおさかなは？" },
	};
	// ※ ここを好きなテキスト＆サイズに書き換えて使う

	if (!talkLines.isEmpty())
	{
		currentIndex = 0;
		m_currentTopic = talkLines[currentIndex]; // ← もう表示には使わないが残してOK
		getData().solvedTasks.clear();
		getData().finalRhymeMatchPercent = 0.0; // スコア機能は無効化
	}
	else
	{
		m_currentTopic = U"お題がありません";
	}

	if (!talkLines.isEmpty())
	{
		currentIndex = 0;
		// 🔽 お題とは無関係に、2〜4 音節のどれかをターゲットにする
		m_currentTopic = talkLines[currentIndex]; // 🎯 最初のお題を保持
		getData().solvedTasks.clear();
		getData().finalRhymeMatchPercent = 0.0; // スコア機能は無効化
	}
	else
	{
		m_currentTopic = U"お題がありません";
	}

	// GIFに関する処理
	// 各フレームの画像と、次のフレームへのディレイ（ミリ秒）をロードする
	gif.read(images, delays);

	// 各フレームの Image から Texure を作成する
	textures = images.map([](const Image& image) { return Texture{ image }; });

	// 画像データはもう使わないため、消去してメモリ消費を減らす
	images.clear();

	m_timer.start(); // タイマー開始

	m_countdownTimer.start();  // カウントダウン開始
	m_showCountdown = true;    // カウントダウンモードON
}

// WriteLyrics::splitSyllables (音節分割関数)
Array<String> WriteLyrics::splitSyllables(const String& text) const
{
	const String smallKanaList = U"ゃゅょぁぃぅぇぉっャュョァィゥェォッ";
	Array<String> result;

	for (size_t i = 0; i < text.length(); ++i)
	{
		String s;
		s += text[i];

		if ((i + 1 < text.length()) && smallKanaList.includes(text[i + 1]))
		{
			s += text[i + 1];
			++i;
		}
		result << s;
	}
	return result;
}

// WriteLyrics::getVowel (母音取得ヘルパー関数) 
char WriteLyrics::getVowel(const String& syllable) const
{
	// 拗音（きゃ、しゅ、てょなど）は最後の母音、撥音/促音は N/Q
	if (syllable == U"ん" || syllable == U"ン") return 'N'; // 撥音
	if (syllable == U"っ" || syllable == U"ッ") return 'Q'; // 促音

	// 小文字（ゃゅょぁぃぅぇぉ）は splitSyllables で前の文字に結合されている前提

	// 結合されている場合、最後の文字が母音を決定する
	// 例: きゃ の 'ゃ' の母音は 'a'
	const String lastChar = syllable.substr(syllable.length() - 1);

	if (lastChar == U"あ" || lastChar == U"か" || lastChar == U"さ" || lastChar == U"た" || lastChar == U"な" || lastChar == U"は" || lastChar == U"ま" || lastChar == U"や" || lastChar == U"ら" || lastChar == U"わ" || lastChar == U"が" || lastChar == U"ざ" || lastChar == U"だ" || lastChar == U"ば" || lastChar == U"ぱ" || lastChar == U"ぁ" || lastChar == U"ゃ") return 'a';
	if (lastChar == U"い" || lastChar == U"き" || lastChar == U"し" || lastChar == U"ち" || lastChar == U"に" || lastChar == U"ひ" || lastChar == U"み" || lastChar == U"り" || lastChar == U"ゐ" || lastChar == U"ぎ" || lastChar == U"じ" || lastChar == U"ぢ" || lastChar == U"び" || lastChar == U"ぴ" || lastChar == U"ぃ") return 'i';
	if (lastChar == U"う" || lastChar == U"く" || lastChar == U"す" || lastChar == U"つ" || lastChar == U"ぬ" || lastChar == U"ふ" || lastChar == U"む" || lastChar == U"ゆ" || lastChar == U"る" || lastChar == U"ぐ" || lastChar == U"ず" || lastChar == U"づ" || lastChar == U"ぶ" || lastChar == U"ぷ" || lastChar == U"ぅ" || lastChar == U"ゅ") return 'u';
	if (lastChar == U"え" || lastChar == U"け" || lastChar == U"せ" || lastChar == U"て" || lastChar == U"ね" || lastChar == U"へ" || lastChar == U"め" || lastChar == U"れ" || lastChar == U"ゑ" || lastChar == U"げ" || lastChar == U"ぜ" || lastChar == U"で" || lastChar == U"べ" || lastChar == U"ぺ" || lastChar == U"ぇ") return 'e';
	if (lastChar == U"お" || lastChar == U"こ" || lastChar == U"そ" || lastChar == U"と" || lastChar == U"の" || lastChar == U"ほ" || lastChar == U"も" || lastChar == U"よ" || lastChar == U"ろ" || lastChar == U"を" || lastChar == U"ご" || lastChar == U"ぞ" || lastChar == U"ど" || lastChar == U"ぼ" || lastChar == U"ぽ" || lastChar == U"ぉ" || lastChar == U"ょ") return 'o';

	// ひらがな・カタカナ以外の文字（漢字や句読点など）が入ってきた場合のデフォルト
	return 'X'; // 不明な母音として扱う
}

bool WriteLyrics::isHiraganaOnly(const String& text) const
{
	for (const auto& ch : text)
	{
		if (!((U'ぁ' <= ch && ch <= U'ん') || ch == U'ー'))
		{
			return false; // ひらがな以外が含まれている
		}
	}
	return true;
}

// 🎵 「ー」を直前の母音（あいうえお）に変換
String WriteLyrics::replaceChoonWithVowel(const String& text) const
{
	String result;

	for (size_t i = 0; i < text.size(); ++i)
	{
		const char32 ch = text[i];

		if (ch == U'ー' && !result.isEmpty())
		{
			const char vowel = getVowel(String(1, result.back()));

			switch (vowel)
			{
			case 'a': result += U"ア"; break;
			case 'i': result += U"イ"; break;
			case 'u': result += U"ウ"; break;
			case 'e': result += U"エ"; break;
			case 'o': result += U"オ"; break;
			case 'N': result += U"ん"; break;
			case 'Q': result += U"っ"; break;
			default:  result += U"ら"; break; // 不明時はそのまま
			}
		}
		else
		{
			result += ch;
		}
	}

	return result;
}

void WriteLyrics::update()
{

	// ✅ 全体集計 + 遷移（韻スコアは使わない）
	auto finalizeAndExit = [&]()
		{
			// ここでは歌詞の再構成だけ行う
			String reconstructedLyrics;
			for (const auto& task : getData().solvedTasks)
			{
				reconstructedLyrics += task.userInput + U"\n";
			}
			getData().fullLyrics = reconstructedLyrics;

			// スコア機能は無効なので 0 固定
			getData().finalRhymeMatchPercent = 0.0;

			changeScene(U"VocalSynthesis", 0.3s);
		};

	if (m_showCountdown)
	{
		double elapsed = m_countdownTimer.s();

		if (elapsed >= m_countdownDuration)
		{
			// カウントダウン終了
			m_showCountdown = false;
			m_timer.restart(); // ゲームタイマー開始
		}

		return; // カウントダウン中は何もしない
	}

	// talkLinesが空なら何もしない
	if (talkLines.isEmpty())
	{
		Print << U"⚠️ お題がありません。";
		return;
	}

	// カウントダウン
	const int32 remaining = m_timeLimit - static_cast<int32>(m_timer.s());

	// ── タイムアップ分岐 ──
	if (remaining <= 0)
	{
		const Array<String> targetSyllables = splitSyllables(talkLines[currentIndex]);

		// 🔽 タイムアップ時も「現在のお題で要求されていた 2〜4 音節分」の「ら」で埋める
		String autoAnswer(4, U'ら');

		m_errorMessage.clear();

		getData().solvedTasks << SolvedTask{
			.phrase = talkLines[currentIndex],
			.syllables = targetSyllables,
			.userInput = autoAnswer,
			.userSyllables = splitSyllables(autoAnswer),
			.score = 0.0,  // スコア機能は無効
			.rhymeMatchPercent = 0.0,
			.matchesCount = 0
		};

		++currentIndex;

		if (currentIndex < talkLines.size())
		{
			m_currentTopic = talkLines[currentIndex];
			m_textState.text.clear();
			m_timer.restart();
		}
		else
		{
			// ✅ 最後のお題がタイムアップでも集計してから遷移
			finalizeAndExit();
		}
		return;
	}

	if (m_textState.enterKey)
	{
		m_textState.enterKey = false;

		// ✅ ひらがな判定
		if (!isHiraganaOnly(m_textState.text))
		{
			m_errorMessage = U"⚠️ ひらがなのみで入力してください";
			m_textState.active = true;
			return; // ← 処理を中断（送信しない）
		}

		// ✅ 先頭が長音ならエラー
		if (!m_textState.text.isEmpty() && m_textState.text.front() == U'ー')
		{
			m_errorMessage = U"⚠️ 言葉の先頭を「ー」から始めることはできません";
			m_textState.active = true;
			return;
		}

		// ✅ 促音「っ」が含まれていたらエラー
		if (m_textState.text.includes(U'っ'))
		{
			m_errorMessage = U"⚠️ 「っ」を入力することはできません";
			m_textState.active = true;
			return;
		}

		// ✅ 「ー」を直前の母音に変換
		String normalizedText = replaceChoonWithVowel(m_textState.text);

		// 音節分割
		Array<String> syllables2 = splitSyllables(normalizedText);
		const size_t s = syllables2.size();

		// 🔽 音節数チェック（2〜4音節のみ許可）
		if (s < 2 || s > 4)
		{
			m_errorMessage = U"⚠️ 2〜4 音節で入力してください\n（いま "
				+ Format(s) + U" 音節）";
			m_textState.active = true;
			return;
		}

		// 🔽 ここから「必ず4音節に変換」する処理
		// 2音節: 最後の音節の母音を1音節として追加し、その後ろに「が」
		// 3音節: 「が」を追加
		// 4音節: 何もしない

		// 母音 → ひらがな文字への変換ヘルパー
		auto vowelToKana = [](char v)->String
			{
				switch (v)
				{
				case 'a': return U"ァ";
				case 'i': return U"ィ";
				case 'u': return U"ゥ";
				case 'e': return U"ェ";
				case 'o': return U"ォ";
				case 'N': return U"ん"; // 念のため
				case 'Q': return U"っ";
				default:  return U"あ"; // デフォルト（ほぼ来ない想定）
				}
			};

		String finalText = normalizedText;
		Array<String> finalSyllables = syllables2;

		if (s == 2)
		{
			const char v = getVowel(syllables2.back());
			const String vowelKana = vowelToKana(v);

			if (currentIndex <= 2)
			{
				// 🎲 1〜3問目 & 6問目以降：従来どおり「母音 + が」
				finalText += vowelKana;
				finalText += U"が";

				finalSyllables << vowelKana;
				finalSyllables << U"が";
			}
			else // 4〜5問目 (currentIndex == 3 or 4)
			{
				// 🎯 4〜5問目：母音をもう1つ追加（「が」は付けない）
				finalText += vowelKana;
				finalText += vowelKana;

				finalSyllables << vowelKana;
				finalSyllables << vowelKana;
			}
		}
		else if (s == 3)
		{
			if (currentIndex <= 2 || currentIndex >= 5)
			{
				// 🎲 1〜3問目 & 6問目以降：従来どおり「が」を追加
				finalText += U"が";
				finalSyllables << U"が";
			}
			else // 4〜5問目
			{
				// 🎯 4〜5問目：末尾母音を1つ追加（「が」は付けない）
				const char v = getVowel(syllables2.back());
				const String vowelKana = vowelToKana(v);

				finalText += vowelKana;
				finalSyllables << vowelKana;
			}
		}
		// s == 4 の場合は finalText / finalSyllables をそのまま使う

		// ここに来た時点で finalSyllables.size() は必ず 4 になる

		const Array<String> targetSyllables = splitSyllables(talkLines[currentIndex]);

		// ✅ 韻スコアは使わないので、0 を入れておくだけ
		getData().solvedTasks << SolvedTask{
			.phrase = talkLines[currentIndex],
			.syllables = targetSyllables,
			.userInput = finalText,       // ← 4音節に変換後の文字列
			.userSyllables = finalSyllables,  // ← 4音節の配列
			.score = 0.0,
			.rhymeMatchPercent = 0.0,
			.matchesCount = 0
		};

		// 🎉 エラーは解消されたので消す
		m_errorMessage.clear();

		// 次のお題へ
		++currentIndex;
		m_timer.restart(); // タイマー再スタート

		if (currentIndex < talkLines.size())
		{
			m_currentTopic = talkLines[currentIndex]; // 🎯 表示中お題を更新
		}
		else
		{
			finalizeAndExit();
		}

		m_textState.text.clear();
		m_textState.active = true;
	}
}

void WriteLyrics::draw() const
{
	ClearPrint();

	// === カウントダウン表示 ===
	if (m_showCountdown)
	{
		double elapsed = m_countdownTimer.s();
		int remaining = static_cast<int>(Math::Ceil(m_countdownDuration - elapsed));

		String countdownText;
		if (remaining > 0)
			countdownText = Format(remaining);
		else
			countdownText = U"START!";

		background.scaled(1.05).drawAt(Scene::Center());
		m_font(countdownText).drawAt(Scene::Center().movedBy(0, 80), kogetyaColor);
		m_font(getData().songTitle).drawAt(70, Scene::Center().movedBy(0, -120), kogetyaColor);

		return; // カウントダウン中は他を描かない
	}

	// アニメーションの経過時間
	double t = Scene::Time();

	// 経過時間と各フレームのディレイに基づいて、何番目のフレームを描けばよいかを計算する
	size_t frameIndex = AnimatedGIFReader::GetFrameIndex(t, delays);

	textures[frameIndex].drawAt(Scene::Center());

	frame.draw();

	// 🎯 お題を中央に大きく描画
	if (currentIndex < m_topics.size())
	{
		// 1〜5問目: プログラム内で決めたお題を表示
		const auto& topic = m_topics[currentIndex];

		m_font(topic.text)
			.drawAt(topic.fontSize,
				Vec2{ Scene::Center().x, Scene::Center().y - 105 },
				kogetyaColor);
	}
	else if (!talkLines.isEmpty() && currentIndex < talkLines.size())
	{
		// 6問目以降 or topics が足りない場合:
		// これまで通り vvproj からのテキストを表示（保険）
		m_font(talkLines[currentIndex])
			.drawAt(Scene::Center().x, Scene::Center().y - 105, kogetyaColor);
	}


	// 💬 テキストボックスを下中央に配置
	constexpr double textBoxWidth = 200.0;
	constexpr double yPos = 594.0;
	const double xPos = (Scene::Width() - textBoxWidth) / 2.0;
	const Vec2 textBoxPos{ xPos, yPos };

	// --- スケーリング係数 ---
	const double scale = 4.0;

	// --- マウス入力と描画に同じスケールを適用 ---
	{
		const Transformer2D transformer(Mat3x2::Scale(scale, Scene::Center()), TransformCursor::Yes);

		// スケールが適用された範囲で描画＆マウス操作
		SimpleGUI::TextBox(m_textState, textBoxPos, textBoxWidth);
	}

	// 🧮 残りお題カウンターを左上に表示
	if (!talkLines.isEmpty())
	{
		// 例: "1 / 3" のような形式
		const String progressText = U"{} / {}"_fmt(currentIndex + 1, talkLines.size());

		m_font(progressText).draw(62, Vec2{ 80, 120 }, kogetyaColor);
	}

	// ⏰ カウントダウン（右上）
	const int32 remaining = Max(0, m_timeLimit - static_cast<int32>(m_timer.s()));
	const String timeText = U"{}"_fmt(remaining);
	const Vec2 pos{ Scene::Width() - 155, 145 };
	m_font(timeText).drawAt(97, pos, (remaining <= 3 ? Palette::Red : kogetyaColor));

	// ❗ 入力エラー表示（あれば）
	if (!m_errorMessage.isEmpty())
	{
		// 位置や大きさは好みで微調整してOK
		result_font(m_errorMessage)
			.draw(22, Vec2{ 40, 480 }, Palette::Red);
		// あるいは中央寄せにしたいなら：
		// result_font(m_errorMessage).drawAt(22, Scene::Center().movedBy(0, 140), Palette::Red);
	}
	/*
	// 🔁 直前のお題と回答を表示（2問目以降のみ）
	if (currentIndex > 0)
	{
		const auto& prevTask = getData().solvedTasks[currentIndex - 1];
		const double yBase = 520;

		result_font(U"　　　前のお題：" + prevTask.phrase)
			.draw(22, Vec2{ 40, yBase }, Palette::White);

		result_font(U"　あなたの回答：" + prevTask.userInput)
			.draw(22, Vec2{ 40, yBase + 30 }, Palette::Skyblue);
	}
	*/
}
