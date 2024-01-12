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
#include <iomanip>

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

class GPS {
private:
#ifdef SAMP
    HANDLE hThread = NULL;
#endif

    // Config
    Config cfg;

    //Logging stuff
    unsigned short logLines;
    std::ofstream logfile;
    void Log(std::string val);
    bool once;

    // These variables will be used for the gps route following the target blip set by the player
    float targetDistance;
    short targetNodesCount;

    CVector destVec;

    CNodeAddress t_ResultNodes[MAX_NODE_POINTS];
    RwIm2DVertex t_LineVerts[MAX_NODE_POINTS * 4];

    // These will be used for mission objectives
    float missionDistance;
    short missionNodesCount;

    CNodeAddress m_ResultNodes[MAX_NODE_POINTS];
    RwIm2DVertex m_LineVerts[MAX_NODE_POINTS * 4];

    char pathNodesToStream[1024];
    int pathNodes[50000];

    CVector PrevPos;
    CVector PrevDest;

    CVector PlayerPos;
    void UpdatePlayerPos();

    bool renderMissionRoute;
    bool renderTargetRoute;

    CVector targetTracePos;

    tRadarTrace mTrace;

    CRGBA CurrentColor;

#ifdef SAMP
    static LPVOID WINAPI init(LPVOID* lpParam);
#endif

    void DrawRadarOverlayHandle();
    void GameEventHandle();
    void DrawHudEventHandle();

    void Run();

    bool CheckBMX(CPed* player);

    const char* VectorToString(std::vector<tRadarTrace>& vec);

    bool NavEnabled(CPed* player);

    CRGBA SetupColor(short color, bool friendly);

    void Setup2dVertex(RwIm2DVertex& vertex, float x, float y, CRGBA clr);

    // Self explanatory.
    void calculatePath(
        CVector destPosn,
        short& nodesCount,
        CNodeAddress* resultNodes,
        float& gpsDistance
    );

    void renderPath(
        CVector tracePos, 
        short color, 
        bool friendly, 
        short& nodesCount, 
        CNodeAddress* resultNodes, 
        float& gpsDistance, 
        RwIm2DVertex* lineVerts
    );

    void renderMissionTrace(tRadarTrace trace);

public:
    GPS();
    ~GPS();
} GPSLineRedux;