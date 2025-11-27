# include "Scene1.hpp"

Scene1::Scene1(const InitData& init)
	: IScene{ init }
{
	// GIFに関する処理
	// 各フレームの画像と、次のフレームへのディレイ（ミリ秒）をロードする
	gif.read(images, delays);

	// 各フレームの Image から Texure を作成する
	textures = images.map([](const Image& image) { return Texture{ image }; });

	// 画像データはもう使わないため、消去してメモリ消費を減らす
	images.clear();

	m_timer.start(); // タイマー開始

	/*
	Console << U"===== Scene3: 受け取った結果 =====";

	// 共有データを直接参照して表示
	for (const auto& t : getData().solvedTasks)
	{
		Console << U"お題：" << t.phrase;
		Console << U"  音節リスト：" << t.syllables; // 例: [え, ぶ, り, で, い]
		Console << U"入力：" << t.userInput;
		Console << U"  入力の音節リスト：" << t.userSyllables;
	}
	*/

	const JSON originalVV = JSON::Load(getData().vvprojPath);
	const String base = FileSystem::BaseName(getData().vvprojPath);
	m_baseName = base;
	const String singerName = U"ずんだもん";
	getData().SingingNames << singerName;
	const int i = 0; // ずんだもん1人のときのインデックス
	const int32  spkID = 3003;	// ずんだもん（ノーマル）
	const int32  talkSpkID = spkID - 3000;

	// 歌詞差し替えした vvproj を作って保存
	JSON parodyVV = VOICEVOX::ApplyParodyLyrics(
		originalVV,
		getData().solvedTasks
	);

	// 歌詞差し替え済み JSON (parodyVV) を一時保存
	FilePath tmpPath = U"tmp/tmp_parody_" + FileSystem::BaseName(getData().vvprojPath) + U".vvproj";
	parodyVV.save(tmpPath);

	// 全歌詞を取得
	auto allLyricMoras = VOICEVOX::ExtractSongLyrics(tmpPath);

	// そのまま連結して 1 本の歌詞に
	String full = allLyricMoras.join(U"");   // モーラ連結
	//Console << U"🎵 最終歌詞: " + full;
	getData().fullLyrics = full; // 共有データへ保存

	// 一時vvproj
	FilePath vvTmp = U"tmp/tmp_modified_" + base + U"_track" + Format(i + 1) + U".vvproj";
	parodyVV.save(vvTmp);

	// ③ スコアJSONへの変換は、元vvprojではなくvvTmpを使う
	FilePath score = U"tmp/tmp_" + base + U"_track" + Format(i + 1) + U".json";
	VOICEVOX::ConvertVVProjToScoreJSON(vvTmp, score, i);

	FilePath songwav = U"Voice/" + base + U"-ずんだもん（ノーマル）_track" + Format(i + 1) + U".wav";

	int keyShift = VOICEVOX::GetKeyAdjustment(U"ずんだもん", U"ノーマル");

	// あとで再生に使うため、パスを記録しておく
	m_songWavPath = songwav;
	m_scorePath = score;
	m_baseName = base;

	// ✅ 非同期タスクとして合成を実行
	m_isLoading = true;
	m_timer.restart();

	m_task = Async([=]() {
		return VOICEVOX::SynthesizeFromJSONFileWrapperSplit(score, songwav, spkID, getData().baseURL, 2500, keyShift);
	});
}

void Scene1::update()
{
	// 非同期処理が終わったか？
	if (m_isLoading && m_task.isReady())
	{
		const bool success = m_task.get(); // 結果を取得
		m_isLoading = false;

		if (success)
		{
			// 🎵 音声と伴奏をロード
			Audio songAudio{ m_songWavPath, Loop::No };
			FileSystem::Remove(m_scorePath);
			Audio inst{ U"Inst/" + m_baseName + U".mp3", Loop::No };

			//Console << U"「" + m_baseName + U"」の再生準備が完了しました。";

			// ✅ 共有データへ保存
			getData().charCount = 1;
			getData().SingerNames = { U"ずんだもん" };
			getData().StyleNames = { U"ノーマル" };
			getData().songAudio = { songAudio };
			getData().instAudio = inst;
			getData().vvprojPath = getData().vvprojPath;
			getData().songTitle = FileSystem::BaseName(getData().vvprojPath);
			getData().readyToPlay = true;
		}

		//Console << U"音声合成が完了しました";
		changeScene(U"Result", 0.3s);
	}
}

void Scene1::draw() const
{

	//GIFアニメーションの描画
	ClearPrint();

	// フレーム数

	//Print << textures.size() << U" frames";

	// 各フレームのディレイ（ミリ秒）一覧
	//Print << U"delays: " << delays;

	// アニメーションの経過時間
	double t = Scene::Time();

	// 経過時間と各フレームのディレイに基づいて、何番目のフレームを描けばよいかを計算する
	size_t frameIndex = AnimatedGIFReader::GetFrameIndex(t, delays);

	// 現在のフレーム番号
	//Print << U"frameIndex: " << frameIndex;

	textures[frameIndex].drawAt(Scene::Center());

	m_font(U"ずんだもん が おうた を\n\nれんしゅう しているよ").drawAt(60, Scene::Center().x,Scene::Center().y-200, kogetyaColor);
}
