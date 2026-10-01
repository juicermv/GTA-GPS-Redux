#pragma once
#include <unordered_map>
#include "CRGBA.h"
#include "mini/ini.h"

namespace util
{
	struct Config
	{
		bool ENABLE_BMX, ENABLE_MOVING, ENABLE_WATER_GPS, RESPECT_LANE_DIRECTION, ENABLE_DISTANCE_TEXT,
			DISTANCE_UNITS = 0;
		bool LOGFILE_ENABLED = 0;
		bool ENABLE_CUSTOM_CLRS = 0;
		float GPS_LINE_WIDTH, DISABLE_PROXIMITY = 0.0f;

		// Keyed by blip color index (see eBlipColour). Populated from the
		// legacy named [Custom Colors] keys (red/green/.../cyan -> 0-6) and
		// from any "colorN=" key, so a mod can register a color for a blip
		// index outside vanilla's 0-8 range without any code changes here.
		std::unordered_map<unsigned int, CRGBA> CUSTOM_COLORS;
		CRGBA GPS_LINE_CLR = {180, 24, 24, 255};

		Config(const char *filename);
	};
} // namespace util