#include "Config.h"
#include <algorithm>
#include <cctype>
#include <charconv>

namespace util
{
	// Parses "R, G, B, A" into out. Returns false (leaving out untouched) on
	// anything malformed instead of throwing - this runs on user-editable
	// ini text, so a typo or a missing value must not be able to crash the
	// plugin during Config's construction at DLL load.
	bool TryParseColor(std::string in, CRGBA &out)
	{
		in.erase(std::ranges::remove_if(in, isspace).begin(), in.end());

		unsigned char *channels[4] = {&out.r, &out.g, &out.b, &out.a};
		for (unsigned char i = 0; i < 4; i++)
		{
			size_t pos = in.find(',');
			std::string token = in.substr(0, pos);

			int value = -1;
			auto result = std::from_chars(token.data(), token.data() + token.size(), value);
			if (result.ec != std::errc() || value < 0 || value > 255)
				return false;

			*channels[i] = static_cast<unsigned char>(value);

			if (pos == std::string::npos)
				return i == 3;

			in.erase(0, pos + 1);
		}
		return true;
	}

	// Only touches CUSTOM_COLORS when the key is actually present -
	// ini[section][key] auto-creates an empty string for a missing key,
	// which used to make color parsing throw on any incomplete ini.
	static void LoadNamedColor(mINI::INIStructure &ini, const std::string &key, unsigned int blipColor,
							   std::unordered_map<unsigned int, CRGBA> &out)
	{
		if (!ini["Custom Colors"].has(key))
			return;

		CRGBA color;
		if (TryParseColor(ini["Custom Colors"].get(key), color))
			out[blipColor] = color;
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
			if (ini["Custom Colors"].has("waypoint"))
				TryParseColor(ini["Custom Colors"].get("waypoint"), GPS_LINE_CLR);

			LoadNamedColor(ini, "red", 0, CUSTOM_COLORS);
			LoadNamedColor(ini, "green", 1, CUSTOM_COLORS);
			LoadNamedColor(ini, "blue", 2, CUSTOM_COLORS);
			LoadNamedColor(ini, "white", 3, CUSTOM_COLORS);
			LoadNamedColor(ini, "yellow", 4, CUSTOM_COLORS);
			LoadNamedColor(ini, "purple", 5, CUSTOM_COLORS);
			LoadNamedColor(ini, "cyan", 6, CUSTOM_COLORS);

			// Any "colorN=" key registers/overrides an arbitrary blip color
			// index, so a mod can add colors beyond vanilla's 0-8 range
			// without needing a named slot here.
			for (auto const &entry : ini["Custom Colors"])
			{
				const std::string &key = entry.first;
				if (key.rfind("color", 0) != 0)
					continue;

				std::string indexPart = key.substr(5);
				if (indexPart.empty() ||
					!std::ranges::all_of(indexPart, [](unsigned char c) { return std::isdigit(c) != 0; }))
					continue;

				unsigned int index = 0;
				std::from_chars(indexPart.data(), indexPart.data() + indexPart.size(), index);

				CRGBA color;
				if (TryParseColor(entry.second, color))
					CUSTOM_COLORS[index] = color;
			}
		}

		/* Log */
		LOGFILE_ENABLED = static_cast<bool>(std::atoi(ini["Misc"]["enableLog"].c_str()));

		file.write(ini);
	}
} // namespace util