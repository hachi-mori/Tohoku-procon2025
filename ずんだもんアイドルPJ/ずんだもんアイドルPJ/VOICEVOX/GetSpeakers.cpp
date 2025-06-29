# include "../stdafx.h"

namespace VOICEVOX
{
	[[nodiscard]]
	Array<Speaker> GetSpeakers(const Duration timeout)
	{
		constexpr URLView url = U"http://localhost:50021/speakers";

		AsyncHTTPTask task = SimpleHTTP::GetAsync(url, {});

		Stopwatch stopwatch{ StartImmediately::Yes };

		while (not task.isReady())
		{
			if (timeout <= stopwatch)
			{
				task.cancel();
				return{};
			}

			System::Sleep(1ms);
		}

		if (not task.getResponse().isOK())
		{
			return{};
		}

		const JSON json = task.getAsJSON();
		Array<Speaker> speakers;

		for (auto&& [i, speaker] : json)
		{
			Speaker s;
			s.name = speaker[U"name"].get<String>();

			for (auto&& [k, style] : speaker[U"styles"])
			{
				Speaker::Style st;
				st.name = style[U"name"].get<String>();
				st.id = style[U"id"].get<int32>();
				s.styles.push_back(st);
			}

			speakers.push_back(s);
		}

		return speakers;
	}
}

