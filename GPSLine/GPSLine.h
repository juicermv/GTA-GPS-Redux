/*
    Plugin-SDK (Grand Theft Auto) source file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <chrono>
#include <ctime>
#include <vector>
#include <algorithm>
#include <limits>

#include "inipp.h"

#include "plugin.h"
#include "RenderWare.h"
#include "common.h"
#include "CMenuManager.h"
#include "CRadar.h"
#include "CGeneral.h"
#include "CWorld.h"
#include "CPools.h"
#include "CVehicle.h"
#include "CPed.h"
#include "CObject.h"
#include "CPickup.h"
#include "CPickups.h"
#include "CTheScripts.h"
#include "CHudColours.h"
#include "RenderWare.h"
#include "Color.h"
#include "CFont.h"
#include "d3d9.h"

/*
    #define MAX_NODE_POINTS 50000
    #define GPS_LINE_WIDTH  4.0f
    #define GPS_LINE_R  180
    #define GPS_LINE_G  24
    #define GPS_LINE_B  24
    #define GPS_LINE_A  255
    #define MAX_TARGET_DISTANCE 10.0f
*/

class GPSLine {
private:
    // Ini Config stuff
    inipp::Ini<char> iniParser;
    std::ifstream iniFile;

    // Config values
    static const inline int GPSLine::MAX_NODE_POINTS = 5000;
    static inline float GPSLine::GPS_LINE_WIDTH = -1;
    static inline short GPSLine::GPS_LINE_R = -1;
    static inline short GPSLine::GPS_LINE_G = -1;
    static inline short GPSLine::GPS_LINE_B = -1;
    static inline short GPSLine::GPS_LINE_A = -1;
    static inline float GPSLine::DISABLE_PROXIMITY = -1;
    static inline bool GPSLine::ENABLE_BMX = -1;
    static inline bool GPSLine::ENABLE_MOVING = -1;
    
    // Custom Colors config
    static inline bool GPSLine::ENABLE_CUSTOM_CLRS = -1;
    static inline CRGBA 
        GPSLine::CC_RED,
        GPSLine::CC_GREEN,
        GPSLine::CC_BLUE,
        GPSLine::CC_WHITE,
        GPSLine::CC_PURPLE,
        GPSLine::CC_YELLOW,
        GPSLine::CC_CYAN
    ;

    //Logging stuff
    int GPSLine::logLines;
    std::ofstream GPSLine::logfile;
    void GPSLine::Log(std::string val);
    bool once;

    // These variables will be used for the gps route following the target blip set by the player
    bool GPSLine::targetRouteShown;
    float GPSLine::targetDistance;
    short GPSLine::targetNodesCount;

    CNodeAddress GPSLine::t_ResultNodes[MAX_NODE_POINTS];
    CVector2D GPSLine::t_NodePoints[MAX_NODE_POINTS];
    RwIm2DVertex GPSLine::t_LineVerts[MAX_NODE_POINTS * 4];

    // These will be used for mission objectives
    bool GPSLine::missionRouteShown;
    float GPSLine::missionDistance;
    short GPSLine::missionNodesCount;

    CNodeAddress GPSLine::m_ResultNodes[MAX_NODE_POINTS];
    CVector2D GPSLine::m_NodePoints[MAX_NODE_POINTS];
    RwIm2DVertex GPSLine::m_LineVerts[MAX_NODE_POINTS * 4];

    char GPSLine::pathNodesToStream[1024];
    int GPSLine::pathNodes[50000];

    bool GPSLine::CheckBMX();

    const char* GPSLine::VectorToString(std::vector<tRadarTrace>& vec);

    CRGBA GPSLine::ExtractColorFromString(std::string in);

    void GPSLine::Setup2dVertex(RwIm2DVertex& vertex, float x, float y, short color, unsigned char appearance, unsigned char bright, bool friendly);

    // Self explanatory.
    void GPSLine::calculatePath(CVector destPosn, short& nodesCount, CNodeAddress* resultNodes, CVector2D* nodePoints, float& gpsDistance);

    void GPSLine::renderPath(short color, unsigned char appearance, unsigned char bright, bool friendly, short& nodesCount, bool& gpsShown, CNodeAddress* resultNodes, CVector2D* nodePoints, float& gpsDistance, RwIm2DVertex* lineVerts);

    void GPSLine::renderMissionTrace(tRadarTrace trace);

public:

    GPSLine();

} gPSLine;