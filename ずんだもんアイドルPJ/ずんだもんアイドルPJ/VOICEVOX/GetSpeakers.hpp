# pragma once
#include <Siv3D.hpp>

namespace VOICEVOX
{
	struct Speaker
	{
		String name;

		struct Style
		{
			String name;
			int32 id;
		};

		Array<Style> styles;
	};

	[[nodiscard]]
	Array<Speaker> GetSpeakers(const Duration timeout = SecondsF{ 50.0 });
}
