#include "Scene2.hpp"

// Scene2::Scene2 (コンストラクタ) ... 既存コード
Scene2::Scene2(const InitData& init)
	: IScene{ init }, m_textState{}
{
	m_textState.active = true;

	talkLines = VOICEVOX::ExtractTalkUtterances(getData().vvprojPath);

	if (!talkLines.isEmpty())
	{
		currentIndex = 0;
		currentTargetLen = splitSyllables(talkLines[currentIndex]).size();
		m_currentTopic = talkLines[currentIndex]; // 🎯 最初のお題を保持
		getData().solvedTasks.clear();
		getData().finalRhymeMatchPercent = 0.0;
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

}

// Scene2::splitSyllables (音節分割関数) ... 既存コード
Array<String> Scene2::splitSyllables(const String& text) const
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

// Scene2::getVowel (母音取得ヘルパー関数) ... 既存コード
char Scene2::getVowel(const String& syllable) const
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

void Scene2::update()
{
	// talkLinesが空なら何もしない
	if (talkLines.isEmpty())
	{
		Print << U"⚠️ お題がありません。";
		return;
	}

	// カウントダウン
	const int32 remaining = m_timeLimit - static_cast<int32>(m_timer.s());

	// タイムアップ処理
	if (remaining <= 0)
	{
		const Array<String> targetSyllables = splitSyllables(talkLines[currentIndex]);
		const size_t L = targetSyllables.size();

		// 「ららら（音節数）」を生成
		String autoAnswer;
		for (size_t i = 0; i < L; ++i)
		{
			autoAnswer += U"ら";
		}

		// 0% スコアで記録
		getData().solvedTasks << SolvedTask{
			.phrase = talkLines[currentIndex],
			.syllables = targetSyllables,
			.userInput = autoAnswer,
			.userSyllables = splitSyllables(autoAnswer),
			.score = 0.0,
			.rhymeMatchPercent = 0.0,
			.matchesCount = 0
		};

		// 次のお題へ
		++currentIndex;

		if (currentIndex < talkLines.size())
		{
			currentTargetLen = splitSyllables(talkLines[currentIndex]).size();
			m_currentTopic = talkLines[currentIndex];
			m_textState.text.clear();
			m_timer.restart(); // タイマー再スタート
		}
		else
		{
			changeScene(U"Scene1", 0.3s);
		}

		return;
	}

	// 音節数カウント
	const Array<String> syllables = splitSyllables(m_textState.text);

	// 現在のお題情報
	/*
	Print << U"🎯 現在のお題[" << currentIndex << U"]："
		<< talkLines[currentIndex]
		<< U"（必要音節数：" << currentTargetLen << U"）";

	Print << U"⌨️ 入力：" << m_textState.text
		<< U"（音節数：" << syllables.size() << U"）";
		*/
	

	if (m_textState.enterKey)
	{
		m_textState.enterKey = false;

		if (syllables.size() != currentTargetLen)
		{
			Print << U"⚠️ 音節数エラー：" << syllables.size()
				<< U"（必要：" << currentTargetLen << U"）";
		}
		else
		{
			// --- 🎯 スコア計算開始 ---

			// お題の音韻配列とお題の音節数（L）
			const Array<String> targetSyllables = splitSyllables(talkLines[currentIndex]);
			const size_t L = targetSyllables.size(); // == syllables.size() (currentTargetLen)

			size_t matches = 0; // 母音一致数

			// 母音の一致をカウント
			for (size_t i = 0; i < L; ++i)
			{
				const char targetVowel = getVowel(targetSyllables[i]);
				const char userVowel = getVowel(syllables[i]);

				if (targetVowel == userVowel)
				{
					matches++;
				}
			}

			// 調整ノブの定義
			constexpr double k = 0.25; // 長さボーナスの伸び率
			constexpr double C = 2.0;  // 長さボーナスの上限

			// 基本スコア r (一致比率)
			const double r = static_cast<double>(matches) / L;

			// 長さボーナス M(L)
			double M_L = 1.0 + k * (L - 3.0);
			if (L < 3) M_L = 1.0; // 3音韻未満は一律 1.0
			const double lengthBonus = Min(C, M_L);

			// 最終スコア S
			const double score = r * lengthBonus;

			// --- スコア計算終了 ---

			// 🔔 個別のお題のパーセンテージの計算と出力
			const double percentMatch = r * 100.0;

			/*
						Print << U"✅ クリア：" << m_textState.text
				<< U"（" << syllables.size() << U" 音節）";

			*/


			// 🏅 スコア表示とパーセンテージ表示を追加
			//Console << U"🏅 スコア：" << score << U" (一致数:" << matches << U"/" << L << U")";
			//Console << U"💯 韻一致率：" << percentMatch << U"%";

			// ✅ クリア情報を記録
			getData().solvedTasks << SolvedTask{
				.phrase = talkLines[currentIndex],
				.syllables = targetSyllables,
				.userInput = m_textState.text,
				.userSyllables = syllables,
				.score = score,
				.rhymeMatchPercent = percentMatch,
				.matchesCount = matches // 👈 一致数を記録
			};

			// 次のお題へ
			++currentIndex;
			m_timer.restart(); // タイマー再スタート

			if (currentIndex < talkLines.size())
			{
				currentTargetLen = splitSyllables(talkLines[currentIndex]).size();
				m_currentTopic = talkLines[currentIndex]; // 🎯 表示中お題を更新
			}
			else
			{
				Print << U"🎉 全てクリア！";

				// --- 🔔 全体集計ロジックの追加（GameDataに最終結果を保存） ---
				size_t totalSyllables = 0;
				size_t totalMatches = 0;

				// 記録されたすべてのお題の結果を集計
				for (const auto& task : getData().solvedTasks)
				{
					totalSyllables += task.userSyllables.size(); // 各お題の音韻数
					totalMatches += task.matchesCount;           // 各お題の一致数
				}

				double finalRhymeMatchPercent = 0.0;
				if (totalSyllables > 0)
				{
					// (合計一致数 / 合計音韻数) * 100%
					finalRhymeMatchPercent = (static_cast<double>(totalMatches) / totalSyllables) * 100.0;
				}

				// GameData に最終一致率を保存
				getData().finalRhymeMatchPercent = finalRhymeMatchPercent;

				// 結果出力
				//Console << U"🌟 全体結果：" << totalMatches << U" / " << totalSyllables << U" 音韻一致";
				//Console << U"💯 最終一致率：" << finalRhymeMatchPercent << U"%";

				// --- 全体集計ロジックの追加 終了 ---

				// ★ 全クリアしたら次のシーンに遷移
				changeScene(U"Scene1", 0.3s);
			}

			m_textState.text.clear();
		}

		m_textState.active = true;
	}


}

void Scene2::draw() const
{
	//GIFアニメーションの描画
	ClearPrint();

	// フレーム数

	//Print << textures.size() << U" frames";

	// 各フレームのディレイ（ミリ秒）一覧
	// Print << U"delays: " << delays;

	// アニメーションの経過時間
	double t = Scene::Time();

	// 経過時間と各フレームのディレイに基づいて、何番目のフレームを描けばよいかを計算する
	size_t frameIndex = AnimatedGIFReader::GetFrameIndex(t, delays);

	// 現在のフレーム番号
	//Print << U"frameIndex: " << frameIndex;

	textures[frameIndex].drawAt(Scene::Center());

	background.draw();

	// 🎯 お題を中央に大きく描画
	if (!m_currentTopic.isEmpty())
	{
		m_font(m_currentTopic)
			.drawAt(Scene::Center().x, Scene::Center().y - 105, kogetyaColor);
	}

	// 💬 テキストボックスを下中央に配置
	constexpr double textBoxWidth = 200.0;
	constexpr double yPos = 800.0;
	const double xPos = (Scene::Width() - textBoxWidth) / 2.0;
	const Vec2 textBoxPos{ xPos, yPos };
	SimpleGUI::TextBox(m_textState, textBoxPos, textBoxWidth);

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
}
