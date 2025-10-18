#include "Scene2.hpp"

Scene2::Scene2(const InitData& init)
	: IScene{ init }
{
	const auto& common = getData();

	// --- Scene1 からデータを受け取る ---
	charCount = common.charCount;
	SingerNames = common.SingerNames;
	StyleNames = common.StyleNames;
	characterTex = common.characterTex;
	songAudio = common.songAudio;
	talkAudio = common.talkAudio;
	audio_inst = common.instAudio;
	vvprojPath = common.vvprojPath;
	songTitle = common.songTitle;
	talkStartSecs = common.talkStartSecs;
	talkPending = common.talkPending;
	SingingNames = common.SingingNames;

	// --- シーン開始時に音声を自動再生 ---
	audio_inst.setVolume(0.4);
	audio_inst.play();

	for (size_t i = 0; i < charCount; ++i)
	{
		if (!songAudio[i].isEmpty())
		{
			songAudio[i].play();
		}

		// 会話音声があればトーク用フラグをセット
		if (!talkAudio[i].isEmpty())
		{
			talkPending[i] = true;
		}
	}

	isPlaying = true;
	playTimer = 0.0;

	//--------------------------------------
	// キャラ数分ボール生成
	//--------------------------------------
	// 生成数に合わせて先に確保
	balls.reserve(charCount);
	ballColors.reserve(charCount);
	ballLabels.reserve(charCount);

	for (size_t i = 0; i < charCount; ++i)
	{
		const double R = 100.0;
		const double th = (6.28 / Max<size_t>(1, charCount)) * i;
		const Vec2 pos = Scene::Center() + Vec2{ Cos(th), Sin(th) } *R - Vec2{ 1000, 1000 };

		P2Body ball = world.createCircle(
			P2Dynamic,
			pos,
			ballRadius,
			P2Material{ 1.0, 0.6, 0.4 }
		);

		balls << ball;
		ballColors << ColorF{ Random(0.4, 1.0), Random(0.4, 1.0), Random(0.4, 1.0) };

		// ボールのラベルにセット
		ballLabels << SingingNames[i];
	}

	cameraFloorY = camera.getCenter().y;

	// 画面の周りに何枚か置く（−2〜+4 はお好みで）
	for (int k = -2; k <= 4; ++k) {
		spawnGroundTile(k);
	}
}

void Scene2::update()
{
	world.update();

	// --- 最下ボール検出 ---
	size_t lowestIndex = 0;
	if (!balls.isEmpty())
	{
		double lowestY = balls[0].getPos().y;
		for (size_t i = 1; i < balls.size(); ++i) {
			const double y = balls[i].getPos().y;
			if (y > lowestY) {
				lowestY = y;
				lowestIndex = i;
			}
		}

		// カメラ：下方向のみ追従（バウンドで上に戻らない）
		lowestY += cameraPadding;
		cameraFloorY = Max(cameraFloorY, lowestY);

		Vec2 camCenter = camera.getCenter();
		const double followSpeed = 300.0; // なめらかに
		camCenter.y = Math::Lerp(camCenter.y, cameraFloorY, Scene::DeltaTime() * followSpeed);
		camera.setTargetCenter(camCenter);
	}

	// --- 無限床：カメラ下端に合わせてタイルを増殖 ---
	const double viewBottomY = camera.getCenter().y + (Scene::Height() * 0.5);
	const int bottomIndex = static_cast<int>(Math::Floor(viewBottomY / TileHeight));

	// 下方向に先読み生成（+6 は余裕。お好みで）
	for (int k = bottomIndex; k <= bottomIndex + 6; ++k) {
		spawnGroundTile(k);
	}

	// 任意：上に離れたタイルを間引き（メモリ/描画負荷対策）
	const int keepMin = bottomIndex - 6;
	groundTiles.remove_if([&](const GroundPair& gp) {
		if (gp.index < keepMin) {
			createdTiles.erase(gp.index);
			return true;
		}
		return false;
	});

	// --- 最下ボールだけ音量=1、他=0 ---
	if (audio_inst.isPlaying()) {
		for (size_t i = 0; i < charCount; ++i) {
			const double v = (i == lowestIndex) ? 1.0 : 0.0;
			if (!songAudio[i].isEmpty()) songAudio[i].setVolume(v);
			if (!talkAudio[i].isEmpty()) talkAudio[i].setVolume(v);
		}
	}

	camera.update();

	if (isPlaying) {
		playTimer += Scene::DeltaTime();
	}
}

void Scene2::draw() const
{
	Scene::SetBackground(HSV{ 199, 0.99, 0.7 });

	const auto t = camera.createTransformer(); // ★ カメラはここで適用

	// ボール本体 + ラベル
	for (size_t i = 0; i < balls.size(); ++i)
	{
		const Vec2 pos = balls[i].getPos();

		// 物理ボディの形状描画でもOKだが、見た目を統一するなら円で
		Circle{ pos, ballRadius }.draw(ballColors[i]);

		const String& label = ballLabels[i];

		if (!label.isEmpty())
		{
			// 2行化ルール：7文字超えなら4文字+残りに分割
			if (label.length() > 7)
			{
				const String firstLine = label.substr(0, 4);
				const String secondLine = label.substr(4);

				font(firstLine).drawAt(25, pos.movedBy(0, -15), Palette::White);
				font(secondLine).drawAt(25, pos.movedBy(0, 15), Palette::White);
			}
			else
			{
				font(label).drawAt(30, pos, Palette::White);
			}
		}
	}

	// ★ 生成済みタイルをすべて描画
	for (const auto& gp : groundTiles) {
		gp.a.draw(Palette::Skyblue);
		gp.b.draw(Palette::Skyblue);
	}

	camera.draw(Palette::Orange);
}

void Scene2::spawnGroundTile(int tileIndex)
{
	if (createdTiles.count(tileIndex)) return;

	// 画面サイズ想定：横長 1920x1080
	const double halfW = Scene::Width() * 0.5;
	const double marginX = 120.0;       // 左右の余白
	const double centerGap = 220.0;     // 中央の空白帯（左右斜面が交差しないための幅）
	const double baseY = tileIndex * TileHeight;

	// ===== ランダム設定 =====
	// 各タイルごとにわずかに変える（Siv3D標準 Random でOK）
	const double dropL = Random(200.0, 300.0); // 左斜面の高さ
	const double dropR = Random(200.0, 300.0); // 右斜面の高さ
	const double offsetY = Random(-30.0, 30.0); // 全体を上下にずらす

	// ===== 左斜面（↘）：左端 → 中央手前 =====
	const Line leftRamp{
		Vec2{ -halfW + marginX, baseY + offsetY },
		Vec2{ -centerGap, baseY + offsetY + dropL }
	};

	// ===== 右斜面（↙）：右端 → 中央手前 =====
	// 右斜面は左よりも下にずらす（交差防止のため）
	const Line rightRamp{
		Vec2{ +halfW - marginX, baseY + offsetY + dropL + 80.0 },
		Vec2{ +centerGap, baseY + offsetY + dropL + 80.0 + dropR }
	};

	// ===== 物理ボディを生成 =====
	P2Body g1 = world.createLine(P2Static, Vec2{ 0, 0 }, leftRamp);
	P2Body g2 = world.createLine(P2Static, Vec2{ 0, 0 }, rightRamp);

	groundTiles << GroundPair{ g1, g2, tileIndex };
	createdTiles.insert(tileIndex);
}

String extractSingerName(const String& filename)
{
	const size_t left = filename.indexOf(U'-');
	const size_t right = filename.indexOf(U'（');
	if (left != String::npos && right != String::npos && right > left)
	{
		return filename.substr(left + 1, right - (left + 1));
	}
	return U""; // 見つからなかった場合
}
