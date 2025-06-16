#include "gps/PathFinder.h"

namespace gps {

PathFinder::PathFinder(util::Config& cfg) : cfg(cfg) {}

void PathFinder::Calculate(CPed* player, CVector destPosn, short& nodesCount,
                           CNodeAddress* resultNodes, float& gpsDistance) {
    ThePaths.DoPathSearch(
        0, player->GetPosition(), CNodeAddress(), destPosn, resultNodes, &nodesCount,
        MAX_NODE_POINTS, &gpsDistance, 999999.0f, NULL, 999999.0f,
        (player->m_pVehicle->m_nVehicleSubClass != VEHICLE_BOAT &&
         player->m_pVehicle->m_nVehicleSubClass != VEHICLE_BMX && cfg.RESPECT_LANE_DIRECTION),
        CNodeAddress(), false,
        (player->m_pVehicle->m_nVehicleSubClass == VEHICLE_BOAT && cfg.ENABLE_WATER_GPS));
}

} // namespace gps
