#pragma once

/*
	Plugin-SDK (Grand Theft Auto) source file
	Authors: GTA Community. See more here
	https://github.com/DK22Pac/plugin-sdk
	Do not delete this comment block. Respect others' work!
*/

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <ctime>
#include <future>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <mutex>

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

#include "util/Bools.h"
#include "util/Config.h"
#include "util/Logger.h"
#include "util/Misc.h"
#include "util/Render.h"

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
	void DrawHudEventHandle();
	void DrawRadarOverlayHandle();
	void renderMissionTrace(tRadarTrace *trace);
	// Self explanatory.
	void calculatePath(CVector destPosn, short &nodesCount, CNodeAddress *resultNodes, float &gpsDistance);
	void requestTargetPath(CVector destPosn);
	void requestMissionPath(CVector destPosn);
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
	std::array<char, 1024> pathNodesToStream{};
	std::array<int, 50000> pathNodes{};
	std::array<CVector2D, MAX_NODE_POINTS> tmpNodePoints{};
	std::array<CNodeAddress, MAX_NODE_POINTS> t_ResultNodes{};
	std::array<RwIm2DVertex, MAX_NODE_POINTS * 4> t_LineVerts{};
	std::array<CNodeAddress, MAX_NODE_POINTS> m_ResultNodes{};
	std::array<RwIm2DVertex, MAX_NODE_POINTS * 4> m_LineVerts{};
	std::future<void> targetFuture;
	std::future<void> missionFuture;
	std::mutex pathMutex;

  public:
	inline GPS()
	{
		this->Run();
	}
} GPSLineRedux;