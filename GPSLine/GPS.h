/*
	Plugin-SDK (Grand Theft Auto) source file
	Authors: GTA Community. See more here
	https://github.com/DK22Pac/plugin-sdk
	Do not delete this comment block. Respect others' work!
*/
#define _USE_MATH_DEFINES

#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

#include "Config.h"
#include "DistCache.h"

#include "CFont.h"
#include "CGeneral.h"
#include "CHudColours.h"
#include "CMenuManager.h"
#include "CObject.h"
#include "CPed.h"
#include "CPickup.h"
#include "CPickups.h"
#include "CPools.h"
#include "CRadar.h"
#include "CTheScripts.h"
#include "CVehicle.h"
#include "CWorld.h"
#include "Color.h"
#include "RenderWare.h"
#include "common.h"
#include "d3d9.h"
#include "plugin.h"

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

class GPS
{
  private:
	// Config
	Config *cfg = nullptr;

	// Logging stuff
	unsigned short logLines;
	std::ofstream logfile;
	void Log(std::string val);
	bool once;

	// These variables will be used for the gps route following the target blip
	// set by the player
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

	CPed *player;

	bool renderMissionRoute;
	bool renderTargetRoute;

	CVector targetTracePos;

	tRadarTrace *mTrace;

	// Graphics stuff & temp variables moved from heap
	CVector2D tmpNodePoints[MAX_NODE_POINTS];
	CVector2D targetScreen;
	CVector2D tmpPoint;
	CVector2D dir;
	float angle;
	unsigned int vertIndex = 0;
	CVector2D shift[2];
	CPathNode *currentNode;
	CVector nodePosn;

	void DrawRadarOverlayHandle();
	void GameEventHandle();
	constexpr void DrawHudEventHandle();

	void Run();

	constexpr bool CheckBMX(CPed *player);

	const char *VectorToString(std::vector<tRadarTrace> &vec);

	constexpr bool NavEnabled(CPed *player);

	CRGBA SetupColor(short color, bool friendly);

	void Setup2dVertex(RwIm2DVertex &vertex, double x, double y, CRGBA clr);

	// Self explanatory.
	void calculatePath(CVector destPosn, short &nodesCount, CNodeAddress *resultNodes, float &gpsDistance);

	void renderPath(CVector tracePos, short color, bool friendly, short &nodesCount, CNodeAddress *resultNodes,
					float &gpsDistance, RwIm2DVertex *lineVerts);

	void renderMissionTrace(tRadarTrace *trace);

  public:
	GPS();
	~GPS();
} GPSLineRedux;