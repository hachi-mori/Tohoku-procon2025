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
	Texture background{ U"Texture/assets/game_frame.png" };
	Array<String> splitSyllables(const String& text) const;
	Array<String> talkLines;
	size_t currentIndex = 0;      // 現在のお題番号
	size_t currentTargetLen = 0;  // 現在のお題の音節数

	char getVowel(const String& syllable) const; // 👈 【追加】母音取得ヘルパー関数

	//const FilePath fontpath = U"C:/Program Files/Steinberg/UR-C/font/mplus-1c-medium.ttf";
	const FilePath fontpath = U"Texture/Futehodo-MaruGothic.ttf";
	Font m_font{ FontMethod::MSDF, 180 , fontpath };
	String m_currentTopic;     // 現在表示中のお題テキスト

	Stopwatch m_timer;   // カウントダウン用タイマー
	const int32 m_timeLimit = 10; // 各お題の制限時間（秒）

	// GIF アニメーション画像を開く
	const AnimatedGIFReader gif{ U"Texture/assets/game_background2.gif" };
	Array<Image> images;
	mutable Array<int32> delays;
	Array<Texture> textures;

	Color kogetyaColor = { 134,79,9 };
};
