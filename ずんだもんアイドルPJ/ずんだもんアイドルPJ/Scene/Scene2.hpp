# include "../Common.hpp"

//------------------------------------------------------
// Scene2 : ゲームシーン
//------------------------------------------------------
class Scene2 : public App::Scene
{
public:
	Scene2(const InitData& init);

	void update() override;
	void draw() const override;

private:
	mutable TextEditState m_textState;
	String m_message;
	Vec2 m_debugPos;  // ✅ デバッグ文字の位置
	Texture frame{ Resource(U"Texture/assets/game_frame.png") };
	Texture background{ Resource(U"Texture/assets/result_background.png") };
	Array<String> splitSyllables(const String& text) const;
	Array<String> talkLines;
	size_t currentIndex = 0;      // 現在のお題番号
	size_t currentTargetLen = 0;  // 現在のお題の音節数

	char getVowel(const String& syllable) const; // 👈 【追加】母音取得ヘルパー関数
	bool isHiraganaOnly(const String& text) const; // 👈 【追加】ひらがなフィルタ関数
	String replaceChoonWithVowel(const String& text) const; // 👈 【追加】長音記号置換関数

	const FilePath fontpath = Resource(U"Texture/Futehodo-MaruGothic.ttf");
	Font m_font{ FontMethod::MSDF, 180 , fontpath };
	Font result_font{ FontMethod::MSDF, 22 , fontpath };
	String m_currentTopic;     // 現在表示中のお題テキスト

	Stopwatch m_timer;   // カウントダウン用タイマー
	const int32 m_timeLimit = 15; // 各お題の制限時間（秒）

	// GIF アニメーション画像を開く
	const AnimatedGIFReader gif{ Resource(U"Texture/assets/game_background2.gif") };
	Array<Image> images;
	mutable Array<int32> delays;
	Array<Texture> textures;

	Color kogetyaColor = { 134,79,9 };

	Stopwatch m_countdownTimer;   // カウントダウン用タイマー
	bool m_showCountdown = true;  // カウントダウン中フラグ
	double m_countdownDuration = 5.0; // カウントダウン時間（秒）
};
