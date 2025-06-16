#pragma once

/*
	Plugin-SDK (Grand Theft Auto) source file
	Authors: GTA Community. See more here
	https://github.com/DK22Pac/plugin-sdk
	Do not delete this comment block. Respect others' work!
*/

#include <algorithm>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <future>
#include <algorithm>

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
#include "RenderWare.h"
#include "d3d9.h"
#include "plugin.h"
#include "common.h"
#include "Color.h"

#include "util/Config.h"
#include "util/DistCache.h"
#include "util/Logger.h"
#include "util/Render.h"
#include "util/Misc.h"
#include "util/Bools.h"

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
	void Run();
	void GameEventHandle();
	constexpr void DrawHudEventHandle();
	constexpr void DrawRadarOverlayHandle();
	constexpr void renderMissionTrace(tRadarTrace *trace);
	// Self explanatory.
	void calculatePath(CVector destPosn, short &nodesCount, CNodeAddress *resultNodes, float &gpsDistance);
	void renderPath(CVector tracePos, short color, bool friendly, short &nodesCount, CNodeAddress *resultNodes,
					float &gpsDistance, RwIm2DVertex *lineVerts);

	bool renderMissionRoute;
	bool renderTargetRoute;
	float targetDistance;
	short targetNodesCount;
	float angle;
	unsigned int vertIndex = 0;
	// These will be used for mission objectives
	float missionDistance;
	short missionNodesCount;
	util::Config cfg = util::Config("SA.GPS.CONF.ini");
	util::Logger logger = util::Logger(false);
	util::DistCache distCache = util::DistCache();
	CPed *player;
	tRadarTrace *mTrace;
	CPathNode *currentNode;
	CVector destVec;
	CVector targetTracePos;
	CVector2D targetScreen;
	CVector2D tmpPoint;
	CVector2D dir;
	CVector nodePosn;
	CVector2D shift[2];
	char pathNodesToStream[1024];
	int pathNodes[50000];
	CVector2D tmpNodePoints[MAX_NODE_POINTS];
	CNodeAddress t_ResultNodes[MAX_NODE_POINTS];
	RwIm2DVertex t_LineVerts[MAX_NODE_POINTS * 4];
	CNodeAddress m_ResultNodes[MAX_NODE_POINTS];
	RwIm2DVertex m_LineVerts[MAX_NODE_POINTS * 4];

public:
	inline GPS()
	{
		this->Run();
	}
} GPSLineRedux;