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

	// VOICEVOX 関連
	URL baseURL = U"http://localhost:50021";


	// ファイル関連
	Optional<FilePath> vvprojPath;
	const FilePath singQuery = U"Query/SingQuery.json";

	AsyncTask<bool> m_task;    // 非同期タスク
	bool m_isLoading = false;  // ローディング中フラグ
	Stopwatch m_timer;         // 経過時間で画像を切り替える

	mutable FilePath m_songWavPath;
	mutable FilePath m_scorePath;
	mutable  String m_baseName;

	// GIF アニメーション画像を開く
	const AnimatedGIFReader gif{ U"Texture/assets/loding_background.gif" };
	Array<Image> images;
	Array<int32> delays;
	Array<Texture> textures;

	const FilePath fontpath = U"Texture/Futehodo-MaruGothic.ttf";
	Font m_font{ FontMethod::MSDF, 60 , fontpath };
	Color kogetyaColor = { 134,79,9 };
};
