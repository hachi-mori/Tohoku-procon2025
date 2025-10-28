# pragma once
# include <Siv3D.hpp>
struct SolvedTask
{
	String phrase;                  // お題
	Array<String> syllables;        // お題の音節リスト
	String userInput;               // ユーザー入力
	Array<String> userSyllables;    // 入力の音節リスト
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
};

using App = SceneManager<String, GameData>;
