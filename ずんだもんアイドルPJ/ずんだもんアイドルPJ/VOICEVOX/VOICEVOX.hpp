# pragma once
#include <Siv3D.hpp>

namespace VOICEVOX
{
	struct Speaker {
		String name;
		struct Style {
			String name;
			int32 id;
		};
		Array<Style> styles;
	};

	Array<Speaker> GetSpeakers(Duration timeout = SecondsF{ 5.0 });

	// vvproj のトラック数を返す
	size_t GetVVProjTrackCount(const FilePath& vvprojPath);

	// trackIndex 番（0 始まり）のトラックだけ vvproj → Score JSON に変換
	bool   ConvertVVProjToScoreJSON(const FilePath& vvprojPath,
									const FilePath& outJsonPath,
									size_t trackIndex = 0);

	bool SynthesizeFromJSONFile(const FilePath& jsonFilePath, const FilePath& savePath, const URL& synthesisURL, const Duration timeout = SecondsF{ 5.0 });

	bool SynthesizeFromJSONFileWrapper(const FilePath& inputPath, const FilePath& intermediatePath, const FilePath& outputPath, const URL& queryURL, const URL& synthesisURL);

	bool SynthesizeFromJSONFileWrapperSplit(
		const FilePath& inputPath,
		const FilePath& intermediatePath,
		const FilePath& outputPath,
		const URL& queryURL,
		const URL& synthesisURL,
		size_t maxFrames
	);
}
