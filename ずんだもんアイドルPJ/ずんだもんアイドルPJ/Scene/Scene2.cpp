#include "Scene2.hpp"

Scene2::Scene2(const InitData& init)
	: IScene{ init }
{
	m_textState.active = true;

	// vvproj からトークのセリフ配列を取得
	talkLines = VOICEVOX::ExtractTalkUtterances(getData().vvprojPath);

	Print << talkLines.size() << U" 件の発話を抽出しました。";

	// お題が存在する場合、最初の音節数を設定
	if (!talkLines.isEmpty())
	{
		currentIndex = 0;
		currentTargetLen = splitSyllables(talkLines[currentIndex]).size();

		Print << U"🎯 お題[" << currentIndex << U"]：" << talkLines[currentIndex]
			<< U"（音節数 " << currentTargetLen << U"）";
	}
	else
	{
		Print << U"⚠️ トークが空でした。";
	}
}

// 音節分割関数（小文字も考慮）
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

void Scene2::update()
{
	// talkLinesが空なら何もしない
	if (talkLines.isEmpty())
	{
		Print << U"⚠️ お題がありません。";
		return;
	}

	constexpr double textBoxWidth = 200.0;
	constexpr double yPos = 800.0;
	const double xPos = (Scene::Width() - textBoxWidth) / 2.0;
	const Vec2 textBoxPos{ xPos, yPos };

	SimpleGUI::TextBox(m_textState, textBoxPos, textBoxWidth);

	// 音節数カウント
	const Array<String> syllables = splitSyllables(m_textState.text);

	// 現在のお題情報
	Print << U"🎯 現在のお題[" << currentIndex << U"]："
		<< talkLines[currentIndex]
		<< U"（必要音節数：" << currentTargetLen << U"）";

	Print << U"⌨️ 入力：" << m_textState.text
		<< U"（音節数：" << syllables.size() << U"）";

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
			Print << U"✅ クリア：" << m_textState.text
				<< U"（" << syllables.size() << U" 音節）";

			// ✅ クリア情報を記録
			getData().solvedTasks << SolvedTask{
				.phrase = talkLines[currentIndex],
				.syllables = splitSyllables(talkLines[currentIndex]), // ← 音節分割
				.userInput = m_textState.text,
				.userSyllables = syllables // ← 入力の音節配列をそのまま
			};

			// 次のお題へ
			++currentIndex;

			if (currentIndex < talkLines.size())
			{
				currentTargetLen = splitSyllables(talkLines[currentIndex]).size();
				Print << U"➡ 次のお題[" << currentIndex << U"]："
					<< talkLines[currentIndex]
					<< U"（音節数 " << currentTargetLen << U"）";
			}
			else
			{
				Print << U"🎉 全てクリア！";

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
	// 描画は一旦なし（Print のみで確認）
}
