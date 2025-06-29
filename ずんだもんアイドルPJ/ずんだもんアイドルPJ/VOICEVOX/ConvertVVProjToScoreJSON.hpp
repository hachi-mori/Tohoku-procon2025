# pragma once
#include <Siv3D.hpp>

namespace VOICEVOX
{
	// vvproj → VOICEVOX API用 JSON に変換する関数
	void ConvertVVProjToScoreJSON(const s3d::FilePath& vvprojPath,
								   const s3d::FilePath& outJsonPath);
}

