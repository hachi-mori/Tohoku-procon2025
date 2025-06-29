# pragma once
#include <Siv3D.hpp>

namespace VOICEVOX
{
	//
	bool SynthesizeFromJSONFile
	(const FilePath& jsonFilePath,
	const FilePath& savePath,
	const URL& synthesisURL,
	const Duration timeout = SecondsF{ 30.0 });
}

