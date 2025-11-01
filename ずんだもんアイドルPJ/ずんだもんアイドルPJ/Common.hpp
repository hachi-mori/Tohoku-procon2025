# pragma once
# include <Siv3D.hpp>

// 中心座標とサイズでボタンを描画して、クリック判定を返す

inline bool ButtonAt(const Vec2& center, const SizeF& size, const ColorF& color = ColorF{ 0.9, 0.8, 0.6 })
{
	const Vec2 leftTop = (center - size / 2.0);              // ✅ doubleで計算
	const RectF rect{ leftTop, size };                      // ✅ RectFを使う
	const RoundRect roundRect = rect.rounded(6.0);          // ✅ doubleで角丸半径

	if (rect.mouseOver())
	{
		Cursor::RequestStyle(CursorStyle::Hand);
	}

	roundRect.draw(color);

	return rect.leftClicked();
}


struct SolvedTask
{
	String phrase;                  // お題
	Array<String> syllables;        // お題の音節リスト
	String userInput;               // ユーザー入力
	Array<String> userSyllables;    // 入力の音節リスト
	double score = 0.0;             // スコア
	double rhymeMatchPercent = 0.0; // 韻一致率（パーセンテージ）
	size_t matchesCount = 0;        // 👈 【追加】個別のお題で一致した音韻の数
};

// 共有するデータ
struct GameData
{
	// キャラごとの設定情報
	size_t charCount = 0;
	Array<String> SingerNames;
	Array<String> StyleNames;
	Array<Texture> characterTex;

	// 音声データ
	Array<Audio> songAudio;
	Array<Audio> talkAudio;
	Audio instAudio;

	// 楽曲タイトルなど
	FilePath vvprojPath;
	String songTitle;

	// 再生に関する情報
	Array<double> talkStartSecs;
	Array<bool> talkPending;

	// その他フラグ
	bool readyToPlay = false;

	Array<String> SingingNames; // 実際に歌っているキャラの名前

	Array<SolvedTask> solvedTasks; // 成功した結果を保持

	double finalRhymeMatchPercent = 0.0; // 👈 【追加】楽曲を通しての最終一致率

	String fullLyrics; // 🎵 【追加】全ての替え歌をまとめた最終歌詞
};

using App = SceneManager<String, GameData>;
