# pragma once
# include <Siv3D.hpp>
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
};

using App = SceneManager<String, GameData>;
