# include "../stdafx.h"

namespace VOICEVOX
{
	constexpr double kFrameRate = 93.75;

	String MidiToName(int midi)
	{
		static const String names[12] = {
			U"C", U"C#", U"D", U"D#", U"E", U"F",
			U"F#", U"G", U"G#", U"A", U"A#", U"B"
		};
		const int octave = midi / 12 - 1;
		return names[midi % 12] + Format(octave);
	}

	int CalcFrameLen(int64 ticks, double bpm, double tpqn, double& carry)
	{
		double beats = ticks / tpqn;
		double seconds = beats * (60.0 / bpm);
		double rawFrames = seconds * kFrameRate;
		double total = rawFrames + carry;
		int frameLen = static_cast<int>(std::floor(total + 0.5));
		carry = total - frameLen;
		return Max(1, frameLen);
	}


	void ConvertVVProjToScoreJSON(const FilePath& vvprojPath,
								   const FilePath& outJsonPath)
	{
		const JSON src = JSON::Load(vvprojPath);
		const JSON& song = src[U"song"];

		// 解像度(tpqn)
		double tpqn = 480.0;
		if (song[U"tpqn"].isNumber())
		{
			tpqn = song[U"tpqn"].get<double>();
		}

		// BPM の取得 (最初の要素)
		double bpm = 120.0;
		if (song[U"tempos"].isArray())
		{
			for (const auto& tempo : song[U"tempos"].arrayView())
			{
				bpm = tempo[U"bpm"].get<double>();
				break;
			}
		}

		Array<JSON> outNotes;
		double carry = 0.0;

		// 最初に 2-frame の休符
		{
			JSON rest;
			rest[U"frame_length"] = 2;
			rest[U"key"] = JSON();
			rest[U"lyric"] = U"";
			rest[U"notelen"] = U"R";
			outNotes << rest;
		}

		// トラック取得 (最初のトラックのみ)
		if (song[U"tracks"].isObject())
		{
			for (const auto& trackPair : song[U"tracks"])  // オブジェクトのイテレータ
			{
				const JSON& track = trackPair.value;
				if (track[U"notes"].isArray())
				{
					int64 prevEnd = 0;
					for (const auto& n : track[U"notes"].arrayView())
					{
						const int64 pos = n[U"position"].get<int64>();
						const int64 dur = n[U"duration"].get<int64>();
						const int    midi = n[U"noteNumber"].get<int>();
						const String lyric = n[U"lyric"].getString();

						// ギャップ(休符)
						if (const int64 gapTicks = pos - prevEnd; gapTicks > 0)
						{
							int gapFrames = CalcFrameLen(gapTicks, bpm, tpqn, carry);
							JSON gap;
							gap[U"frame_length"] = gapFrames;
							gap[U"key"] = JSON();
							gap[U"lyric"] = U"";
							gap[U"notelen"] = U"R";
							outNotes << gap;
						}

						// 実音符
						int noteFrames = CalcFrameLen(dur, bpm, tpqn, carry);
						JSON note;
						note[U"frame_length"] = noteFrames;
						note[U"key"] = midi;
						note[U"lyric"] = lyric;
						note[U"notelen"] = MidiToName(midi);
						outNotes << note;

						prevEnd = pos + dur;
					}
				}
				break;  // 最初のトラックのみ
			}
		}

		// 最後に 2-frame の休符
		{
			JSON rest;
			rest[U"frame_length"] = 2;
			rest[U"key"] = JSON();
			rest[U"lyric"] = U"";
			rest[U"notelen"] = U"R";
			outNotes << rest;
		}

		// 結果を保存
		JSON result;
		result[U"notes"] = outNotes;
		result.save(outJsonPath);
	}


}
