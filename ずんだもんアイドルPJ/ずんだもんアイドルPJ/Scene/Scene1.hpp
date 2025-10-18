# pragma once
# include "../Common.hpp"

// ゲームシーン
class Scene1 : public App::Scene
{
public:

	Scene1(const InitData& init);

	void update() override;

	void draw() const override;

private:

	// キャラクター上限
	static constexpr size_t kMaxCharacters = 5;

	// vvproj 読み込み後に決まるキャラ数
	size_t charCount = 0;

	// 時間管理
	bool waitingToPlay = false;
	double waitTimer = 0.0;
	double playElapsedSec = 0.0;

	// フォントと画像
	const FilePath fontpath = U"C:/Program Files/Steinberg/UR-C/font/mplus-1c-medium.ttf";
	Font font{ FontMethod::MSDF, 48 , fontpath };
	Texture song_title{ U"Texture/song_title.png" };
	Texture background{ U"Texture/background1.jpg" };

	// 音声・キャラデータ
	Audio audio_inst; // 伴奏
	Array<Texture> characterTex{ kMaxCharacters, Texture{ U"Texture/Character/VOICEVOX/ずんだもん.png" } };
	Array<Audio> songAudio{ kMaxCharacters };
	Array<Audio> talkAudio{ kMaxCharacters };
	Array<double> talkStartSecs{ kMaxCharacters, 0.0 };
	Array<bool> talkPending = Array<bool>(kMaxCharacters, false);

	// VOICEVOX 関連
	Array<String> SingerLabels;
	Array<String> SingerNames;
	Array<String> StyleNames;
	Array<int32>  SingerIDs;
	URL baseURL = U"http://localhost:50021";

	// ListBox UI 管理
	mutable Array<ListBoxState> SingerUI{ kMaxCharacters, ListBoxState{ SingerLabels } };
	Array<Optional<uint64>> prevSel{ kMaxCharacters };

	// レイアウト
	static constexpr double kCenterY = 370;
	static constexpr Vec2 kListOff{ -150, 180 };

	// ファイル関連
	Optional<FilePath> vvprojPath;
	const FilePath singQuery = U"Query/SingQuery.json";

	// 定数・パラメータ
	const double kSingingThreshold = 0.00005;

	// エネルギー解析（Main内ラムダを関数化）
	double analyzeEnergy(const Audio& a) const;
};
