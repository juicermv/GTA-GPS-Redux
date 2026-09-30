#include "Config.h"

namespace util
{
	constexpr bool CheckBMX(const Config& cfg, const CPed *player)
	{
		if (cfg.ENABLE_BMX)
			return false;

		return player->m_pVehicle->m_nVehicleSubClass == VEHICLE_BMX;
	}

	constexpr bool CheckBoat(const Config& cfg, const CPed *player)
	{
		if (cfg.ENABLE_WATER_GPS)
			return false;

		return player->m_pVehicle->m_nVehicleSubClass == VEHICLE_BOAT;
	}

	constexpr bool NavEnabled(const Config& cfg, const CPed *player)
	{
		return (player && player->m_pVehicle && player->m_nPedFlags.bInVehicle &&
				player->m_pVehicle->m_nVehicleSubClass != VEHICLE_PLANE &&
				player->m_pVehicle->m_nVehicleSubClass != VEHICLE_HELI && !CTheScripts::bMiniGameInProgress &&
				!CheckBMX(cfg, player) && !CheckBoat(cfg, player));
	}
} // namespace util