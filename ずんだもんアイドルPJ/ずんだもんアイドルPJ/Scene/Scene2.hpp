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
	TextEditState m_textState;
	Font m_font{20};
	String m_message;
	Vec2 m_debugPos;  // ✅ デバッグ文字の位置
	Texture background{ U"Texture/kanjidego.png" };
	Array<String> splitSyllables(const String& text) const;
	Array<String> talkLines;	
	size_t currentIndex = 0;      // 現在のお題番号
	size_t currentTargetLen = 0;  // 現在のお題の音節数
};
