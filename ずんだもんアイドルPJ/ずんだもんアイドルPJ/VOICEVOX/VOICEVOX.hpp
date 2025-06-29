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

	String MidiToName(int midi);
	int CalcFrameLen(int64 ticks, double bpm, double tpqn, double& carry);

	void ConvertVVProjToScoreJSON(const FilePath& vvprojPath, const FilePath& outJsonPath);

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
