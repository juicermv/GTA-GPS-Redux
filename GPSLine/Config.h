#pragma once

#include <fstream>
#include <iostream>

#include "inipp.h"
#include <CRGBA.h>


void ExtractColorFromString(std::string in, CRGBA &out) {
    // Remove whitespace
    in.erase(std::remove_if(in.begin(), in.end(), isspace), in.end());

    bool
        didR = false,
        didG = false,
        didB = false,
        didA = false
        ;

    size_t pos = 0;
    for (unsigned char i = 0; i < 4; i++) {
        pos = in.find(",");

        if (!didR) {
            out.r = (unsigned char)std::stoi(in.substr(0, pos));
            didR = true;
        }
        else if (!didG) {
            out.g = (unsigned char)std::stoi(in.substr(0, pos));
            didG = true;
        }
        else if (!didB) {
            out.b = (unsigned char)std::stoi(in.substr(0, pos));
            didB = true;
        }
        else if (!didA) {
            out.a = (unsigned char)std::stoi(in.substr(0, pos));
            didA = true;
        }

        in.erase(0, pos + 1);
    }
}

struct Config {
    // Config values
    float GPS_LINE_WIDTH = -1;
    short GPS_LINE_R = -1;
    short GPS_LINE_G = -1;
    short GPS_LINE_B = -1;
    short GPS_LINE_A = -1;
    float DISABLE_PROXIMITY = -1;
    bool ENABLE_BMX = -1;
    bool ENABLE_MOVING = -1;
    bool ENABLE_WATER_GPS = -1;

    bool ENABLE_DISTANCE_TEXT = -1;
    short DISTANCE_UNITS = -1;

    // Custom Colors config
    bool ENABLE_CUSTOM_CLRS = -1;
    CRGBA
        CC_RED,
        CC_GREEN,
        CC_BLUE,
        CC_WHITE,
        CC_PURPLE,
        CC_YELLOW,
        CC_CYAN
    ;

    bool LOGFILE_ENABLED = -1;

    static void LoadConfig(const char* filename, Config &config);
};

void Config::LoadConfig(const char* filename, Config &config) {
    inipp::Ini<char> iniParser;
    std::ifstream iniFile = std::ifstream(filename, std::ios::in);

    iniParser.parse(iniFile);
    iniParser.strip_trailing_comments();
    iniParser.interpolate();

    inipp::get_value(iniParser.sections["Navigation Config"], "Navigation line width", config.GPS_LINE_WIDTH);
    inipp::get_value(iniParser.sections["Navigation Config"], "Enable navigation on bicycles", config.ENABLE_BMX);
    inipp::get_value(iniParser.sections["Navigation Config"], "Enable navigation on boats", config.ENABLE_WATER_GPS);
    inipp::get_value(iniParser.sections["Navigation Config"], "Enable navigation for moving targets", config.ENABLE_MOVING);
    inipp::get_value(iniParser.sections["Navigation Config"], "Navigation line removal proximity", config.DISABLE_PROXIMITY);
    inipp::get_value(iniParser.sections["Navigation Config"], "Enable distance display", config.ENABLE_DISTANCE_TEXT);
    inipp::get_value(iniParser.sections["Navigation Config"], "Units", config.DISTANCE_UNITS);

    inipp::get_value(iniParser.sections["Waypoint Config"], "Waypoint line red", config.GPS_LINE_R);
    inipp::get_value(iniParser.sections["Waypoint Config"], "Waypoint line green", config.GPS_LINE_G);
    inipp::get_value(iniParser.sections["Waypoint Config"], "Waypoint line blue", config.GPS_LINE_B);
    inipp::get_value(iniParser.sections["Waypoint Config"], "Waypoint line opacity", config.GPS_LINE_A);

    inipp::get_value(iniParser.sections["Custom Colors"], "Enabled", config.ENABLE_CUSTOM_CLRS);

    inipp::get_value(iniParser.sections["Misc"], "Enable Logfile", config.LOGFILE_ENABLED);

    if (config.ENABLE_CUSTOM_CLRS) {
        std::string buffer;

        inipp::get_value(iniParser.sections["Custom Colors"], "Red", buffer);
        ExtractColorFromString(buffer, config.CC_RED);

        inipp::get_value(iniParser.sections["Custom Colors"], "Green", buffer);
        ExtractColorFromString(buffer, config.CC_GREEN);

        inipp::get_value(iniParser.sections["Custom Colors"], "Blue", buffer);
        ExtractColorFromString(buffer, config.CC_BLUE);

        inipp::get_value(iniParser.sections["Custom Colors"], "White", buffer);
        ExtractColorFromString(buffer, config.CC_WHITE);

        inipp::get_value(iniParser.sections["Custom Colors"], "Yellow", buffer);
        ExtractColorFromString(buffer, config.CC_YELLOW);

        inipp::get_value(iniParser.sections["Custom Colors"], "Purple", buffer);
        ExtractColorFromString(buffer, config.CC_PURPLE);

        inipp::get_value(iniParser.sections["Custom Colors"], "Cyan", buffer);
        ExtractColorFromString(buffer, config.CC_CYAN);
    }

    iniFile.close();
}