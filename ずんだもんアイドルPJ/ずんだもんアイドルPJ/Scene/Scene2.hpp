# pragma once
# include "../Common.hpp"

// タイトルシーン
class Scene2 : public App::Scene
{
public:

	Scene2(const InitData& init);

	void update() override;

	void draw() const override;

private:
	// === Scene1 から引き継ぐデータ ===
	size_t charCount = 0;
	Array<String> SingerNames;
	Array<String> StyleNames;
	Array<Texture> characterTex;

	Array<Audio> songAudio;
	Array<Audio> talkAudio;
	Audio audio_inst;

	FilePath vvprojPath;
	String songTitle;

	Array<double> talkStartSecs;
	Array<bool> talkPending;

	Array<String> SingingNames;

	// --- Physics2D ---
	// 重力加速度 (cm/s^2)
	double Gravity = 980;

	// 2D 物理演算のワールド
	P2World world{ Gravity };
	Array<P2Body> balls;
	Array<Color> ballColors;
	Array<String> ballLabels;

	// 画面サイズに依存（横長 1920x1080 想定）
	static constexpr double marginX = 120.0;  // 画面左右の余白
	static constexpr double centerGapX = 220.0;  // 中央の空白帯（交差防止）
	static constexpr double drop = 260.0;  // 片斜面の落差
	static constexpr double gapY = 80.0;   // 斜面間の縦間隔（交差防止）
	static constexpr double extraY = 120.0;  // タイル間の余白
	static constexpr double TileHeight = drop * 2 + gapY + extraY; // タイルの縦ピッチ

	// [_] 地面 (幅 1200 cm の床）
	const P2Body ground1 = world.createLine(P2Static, Vec2{ 0, 0 }, Line{ -150, -200, 541, -300 });
	const P2Body ground2 = world.createLine(P2Static, Vec2{ 0, 0 }, Line{ -541, 200, 150, 300 });
	// 無限床タイル用
	struct GroundPair { P2Body a, b; int index; };
	Array<GroundPair> groundTiles;
	std::unordered_set<int> createdTiles; // #include <unordered_set>

	// カメラは下方向のみ追従するための下限記録
	double cameraFloorY = 0.0;
	double cameraPadding = 120.0;

	// 生成関数プロトタイプ
	void spawnGroundTile(int tileIndex);

	// 画面の半分の幅・高さ
	const double halfWidth = Scene::Size().x / 2;
	const double halfHeight = Scene::Size().y / 2;

	// 右端の壁
	const P2Body rightwall = world.createLine(P2Static, Vec2{ halfWidth, 0 }, Line{ Vec2{ 0, -halfHeight }, Vec2{ 0, 100000000 } });
	// 左端の壁
	const P2Body leftwall = world.createLine(P2Static, Vec2{ -halfWidth, 0 }, Line{ Vec2{ 0, -halfHeight }, Vec2{ 0, 100000000 } });

	// 2D カメラ
	Camera2D camera{ Vec2{ 0, 0 } };

	Font font{ 24 };
	double ballRadius = 40.0;

	bool isPlaying = false;
	double playTimer = 0.0;

	String trackLabel(size_t i) const;  // songAudio[i] のファイル名（拡張子なし）を返す

};
