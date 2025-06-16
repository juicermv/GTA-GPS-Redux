#include "Config.h"

namespace util
{
    constexpr bool CheckBMX(Config cfg, CPed *player)
    {
        if (cfg.ENABLE_BMX)
            return false;

        return player->m_pVehicle->m_nVehicleSubClass == VEHICLE_BMX;
    }

    constexpr bool NavEnabled(Config cfg, CPed *player)
    {
        return (player && player->m_pVehicle && player->m_nPedFlags.bInVehicle &&
                player->m_pVehicle->m_nVehicleSubClass != VEHICLE_PLANE &&
                player->m_pVehicle->m_nVehicleSubClass != VEHICLE_HELI && !CTheScripts::bMiniGameInProgress &&
                !CheckBMX(cfg, player));
    }
}