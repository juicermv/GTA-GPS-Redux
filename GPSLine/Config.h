#pragma once

#include <fstream>
#include <iostream>

#include "../external/mini/src/mini/ini.h"
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
    float GPS_LINE_WIDTH ;
    float DISABLE_PROXIMITY ;
    bool ENABLE_BMX ;
    bool ENABLE_MOVING ;
    bool ENABLE_WATER_GPS ;

    bool RESPECT_LANE_DIRECTION ;

    bool ENABLE_DISTANCE_TEXT ;
    bool DISTANCE_UNITS ;

    // Custom Colors config
    bool ENABLE_CUSTOM_CLRS ;
    CRGBA
        CC_RED,
        CC_GREEN,
        CC_BLUE,
        CC_WHITE,
        CC_PURPLE,
        CC_YELLOW,
        CC_CYAN
    ;

    CRGBA GPS_LINE_CLR = { 180, 24 ,24 ,255 };

    bool LOGFILE_ENABLED ;

    static void LoadConfig(const char* filename, Config &config);
};

void Config::LoadConfig(const char* filename, Config &config) {
    mINI::INIFile file(filename);

    mINI::INIStructure ini;
    file.read(ini);

    /* Navigation */
    config.RESPECT_LANE_DIRECTION = (bool)std::stoi(ini["Navigation"]["respectTrafficLaneDirection"]);
    config.GPS_LINE_WIDTH = std::stof(ini["Navigation"]["lineWidth"]);
    config.ENABLE_BMX = (bool)std::stoi(ini["Navigation"]["enableOnBicycles"]);
    config.ENABLE_WATER_GPS = (bool)std::stoi(ini["Navigation"]["enableOnBoats"]);
    config.ENABLE_MOVING = (bool)std::stoi(ini["Navigation"]["trackMovingTargets"]);
    config.DISABLE_PROXIMITY = std::stof(ini["Navigation"]["removeRadius"]);

    /* Extras */
    config.ENABLE_DISTANCE_TEXT = (bool)std::stoi(ini["Navigation"]["displayDistance"]);
    config.DISTANCE_UNITS = (bool)std::stoi(ini["Navigation"]["distanceUnits"]);

    /* Custom Colors */
    config.ENABLE_CUSTOM_CLRS = (bool)std::stoi(ini["Custom Colors"]["enabled"]);
    if (config.ENABLE_CUSTOM_CLRS) {
        ExtractColorFromString(ini["Custom Colors"]["waypoint"], config.GPS_LINE_CLR);
        ExtractColorFromString(ini["Custom Colors"]["red"], config.CC_RED);
        ExtractColorFromString(ini["Custom Colors"]["green"], config.CC_GREEN);
        ExtractColorFromString(ini["Custom Colors"]["blue"], config.CC_BLUE);
        ExtractColorFromString(ini["Custom Colors"]["white"], config.CC_WHITE);
        ExtractColorFromString(ini["Custom Colors"]["yellow"], config.CC_YELLOW);
        ExtractColorFromString(ini["Custom Colors"]["purple"], config.CC_PURPLE);
        ExtractColorFromString(ini["Custom Colors"]["cyan"], config.CC_CYAN);
    }

    /* Log */
    config.LOGFILE_ENABLED = (bool)std::stoi(ini["Misc"]["enableLog"]);
}