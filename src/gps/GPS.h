#pragma once

#include <algorithm>
#include <future>
#include <SIMDString.h>

#include "CFont.h"
#include "CPed.h"
#include "CRadar.h"
#include "CTheScripts.h"
#include "plugin.h"

#include "util/Config.h"
#include "util/DistCache.h"
#include "util/Logger.h"
#include "gps/Common.h"
#include "gps/PathFinder.h"

namespace gps {

class GPS {
private:
    void Run();
    void GameEventHandle();
    void DrawHudEventHandle();
    bool CheckBMX(CPed* player) const;
    bool NavEnabled(CPed* player) const;
    void DrawRadarOverlayHandle();
    void renderMissionTrace(tRadarTrace* trace);
    void renderPath(CVector tracePos, short color, bool friendly,
                    short& nodesCount, CNodeAddress* resultNodes,
                    float& gpsDistance, RwIm2DVertex* lineVerts);

    bool renderMissionRoute{};
    bool renderTargetRoute{};
    float targetDistance{};
    short targetNodesCount{};
    float angle{};
    unsigned int vertIndex = 0;

    float missionDistance{};
    short missionNodesCount{};
    util::Config cfg;
    util::Logger logger;
    util::DistCache distCache;
    gps::PathFinder pathFinder;

    CPed* player{};
    tRadarTrace* mTrace{};
    CPathNode* currentNode{};
    CVector destVec{};
    CVector targetTracePos{};
    CVector2D targetScreen{};
    CVector2D tmpPoint{};
    CVector2D dir{};
    CVector nodePosn{};
    CVector2D shift[2]{};
    char pathNodesToStream[1024]{};
    int pathNodes[50000]{};
    CVector2D tmpNodePoints[MAX_NODE_POINTS]{};
    CNodeAddress t_ResultNodes[MAX_NODE_POINTS]{};
    RwIm2DVertex t_LineVerts[MAX_NODE_POINTS * 4]{};
    CNodeAddress m_ResultNodes[MAX_NODE_POINTS]{};
    RwIm2DVertex m_LineVerts[MAX_NODE_POINTS * 4]{};

public:
    GPS();
};

} // namespace gps

extern gps::GPS GPSLineRedux;
