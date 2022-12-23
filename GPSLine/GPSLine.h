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
#include "Config.h"

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

#define MAX_NODE_POINTS 5000

#ifdef SAMP

#include <Windows.h>
#include <Psapi.h>

#pragma comment( lib, "psapi.lib" )
#pragma comment( lib, "kernel32.lib" )


#define E_ADDR_GAMEPROCESS	0x53E981

#pragma pack(push, 1)
typedef struct stOpcodeRelCall
{
    BYTE bOpcode;
    DWORD dwRelAddr;
} OpcodeRelCall;
#pragma pack(pop)
#endif

class GPSLine {
private:
#ifdef SAMP
    HANDLE GPSLine::hThread = NULL;
#endif

    // Config
    Config cfg;

    //Logging stuff
    unsigned short GPSLine::logLines;
    std::ofstream GPSLine::logfile;
    void GPSLine::Log(std::string val);
    bool once;

    // These variables will be used for the gps route following the target blip set by the player
    bool GPSLine::targetRouteShown;
    float GPSLine::targetDistance;
    short GPSLine::targetNodesCount;

    CNodeAddress GPSLine::t_ResultNodes[MAX_NODE_POINTS];
    CVector2D GPSLine::t_NodePoints[MAX_NODE_POINTS];
    float GPSLine::t_NodeHeights[MAX_NODE_POINTS];
    RwIm2DVertex GPSLine::t_LineVerts[MAX_NODE_POINTS * 4];

    // These will be used for mission objectives
    bool GPSLine::missionRouteShown;
    float GPSLine::missionDistance;
    short GPSLine::missionNodesCount;

    CNodeAddress GPSLine::m_ResultNodes[MAX_NODE_POINTS];
    CVector2D GPSLine::m_NodePoints[MAX_NODE_POINTS];
    float GPSLine::m_NodeHeights[MAX_NODE_POINTS];
    RwIm2DVertex GPSLine::m_LineVerts[MAX_NODE_POINTS * 4];

    char GPSLine::pathNodesToStream[1024];
    int GPSLine::pathNodes[50000];

    CVector GPSLine::PrevPos;
    CVector GPSLine::PrevDest;

    CVector GPSLine::PlayerPos;
    void GPSLine::UpdatePlayerPos();

    CRGBA CurrentColor;

#ifdef SAMP
    static LPVOID WINAPI GPSLine::init(LPVOID* lpParam);
#endif

    void GPSLine::DrawRadarOverlayHandle();
    void GPSLine::GameEventHandle();
    void GPSLine::DrawHudEventHandle();

    void GPSLine::Run();

    bool GPSLine::CheckBMX();

    const char* GPSLine::VectorToString(std::vector<tRadarTrace>& vec);

    CRGBA GPSLine::SetupColor(short color, bool friendly, float height);

    void GPSLine::Setup2dVertex(RwIm2DVertex& vertex, float x, float y, CRGBA clr);

    // Self explanatory.
    void GPSLine::calculatePath(
        CVector destPosn,
        short& nodesCount,
        CNodeAddress* resultNodes,
        CVector2D* nodePoints,
        float* nodeHeights,
        float& gpsDistance
    );

    void GPSLine::renderPath(
        CVector tracePos, 
        short color, 
        bool friendly, 
        short& nodesCount, 
        bool& gpsShown, 
        CNodeAddress* resultNodes, 
        CVector2D* nodePoints, 
        float* NodeHeights,
        float& gpsDistance, 
        RwIm2DVertex* lineVerts
    );

    void GPSLine::renderMissionTrace(tRadarTrace trace);

public:
    GPSLine();
    ~GPSLine();
} GPSLineRedux;