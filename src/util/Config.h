#pragma once
#include "mini/ini.h"
#include "CRGBA.h"

namespace util
{
	struct Config
	{
		bool ENABLE_BMX, ENABLE_MOVING, ENABLE_WATER_GPS, RESPECT_LANE_DIRECTION, ENABLE_DISTANCE_TEXT, DISTANCE_UNITS = 0;
		bool LOGFILE_ENABLED = 0;
		bool ENABLE_CUSTOM_CLRS = 0;
		float GPS_LINE_WIDTH, DISABLE_PROXIMITY = 0.0f;

		CRGBA CC_RED, CC_GREEN, CC_BLUE, CC_WHITE, CC_PURPLE, CC_YELLOW, CC_CYAN;
		CRGBA GPS_LINE_CLR = {180, 24, 24, 255};

		Config(const char *filename);
	};
}