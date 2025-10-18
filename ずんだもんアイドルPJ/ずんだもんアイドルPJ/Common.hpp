# pragma once
# include <Siv3D.hpp>
# include "VOICEVOX/VOICEVOX.hpp"

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
};

using App = SceneManager<String, GameData>;
