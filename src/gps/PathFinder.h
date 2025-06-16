#pragma once
#include <CVector.h>
#include <CPed.h>
#include <Paths.h>
#include "gps/Common.h"
#include "util/Config.h"

namespace gps {
class PathFinder {
    util::Config& cfg;
public:
    explicit PathFinder(util::Config& cfg);
    void Calculate(CPed* player, CVector destPosn, short& nodesCount,
                   CNodeAddress* resultNodes, float& gpsDistance);
};
}
