#include "Config.h"

namespace util
{
	void ExtractColorFromString(std::string in, CRGBA &out)
	{
		// Remove whitespace
		in.erase(std::ranges::remove_if(in, isspace).begin(), in.end());

		bool didR = false, didG = false, didB = false, didA = false;

		size_t pos = 0;
		for (unsigned char i = 0; i < 4; i++)
		{
			pos = in.find(',');

			if (!didR)
			{
				out.r = static_cast<unsigned char>(std::stoi(in.substr(0, pos)));
				didR = true;
			}
			else if (!didG)
			{
				out.g = static_cast<unsigned char>(std::stoi(in.substr(0, pos)));
				didG = true;
			}
			else if (!didB)
			{
				out.b = static_cast<unsigned char>(std::stoi(in.substr(0, pos)));
				didB = true;
			}
			else if (!didA)
			{
				out.a = (unsigned char)std::stoi(in.substr(0, pos).c_str());
				didA = true;
			}

			in.erase(0, pos + 1);
		}
	}

	Config::Config(const char *filename)
	{
		mINI::INIFile file(filename);

		mINI::INIStructure ini;
		file.read(ini);

		/* Navigation */
		RESPECT_LANE_DIRECTION = static_cast<bool>(std::atoi(ini["Navigation"]["respectTrafficLaneDirection"].c_str()));
		GPS_LINE_WIDTH = static_cast<float>(std::atof(ini["Navigation"]["lineWidth"].c_str()));
		ENABLE_BMX = static_cast<bool>(std::atoi(ini["Navigation"]["enableOnBicycles"].c_str()));
		ENABLE_WATER_GPS = static_cast<bool>(std::atoi(ini["Navigation"]["enableOnBoats"].c_str()));
		ENABLE_MOVING = static_cast<bool>(std::atoi(ini["Navigation"]["trackMovingTargets"].c_str()));
		DISABLE_PROXIMITY = static_cast<float>(std::atof(ini["Navigation"]["removeRadius"].c_str()));

		/* Extras */
		ENABLE_DISTANCE_TEXT = static_cast<bool>(std::atoi(ini["Extras"]["displayDistance"].c_str()));
		DISTANCE_UNITS = static_cast<bool>(std::atoi(ini["Extras"]["distanceUnits"].c_str()));

		/* Custom Colors */
		ENABLE_CUSTOM_CLRS = static_cast<bool>(std::atoi(ini["Custom Colors"]["enabled"].c_str()));
		if (ENABLE_CUSTOM_CLRS)
		{
			ExtractColorFromString(ini["Custom Colors"]["waypoint"], GPS_LINE_CLR);
			ExtractColorFromString(ini["Custom Colors"]["red"], CC_RED);
			ExtractColorFromString(ini["Custom Colors"]["green"], CC_GREEN);
			ExtractColorFromString(ini["Custom Colors"]["blue"], CC_BLUE);
			ExtractColorFromString(ini["Custom Colors"]["white"], CC_WHITE);
			ExtractColorFromString(ini["Custom Colors"]["yellow"], CC_YELLOW);
			ExtractColorFromString(ini["Custom Colors"]["purple"], CC_PURPLE);
			ExtractColorFromString(ini["Custom Colors"]["cyan"], CC_CYAN);
		}

		/* Log */
		LOGFILE_ENABLED = static_cast<bool>(std::atoi(ini["Misc"]["enableLog"].c_str()));

		file.write(ini);
	}
} // namespace util