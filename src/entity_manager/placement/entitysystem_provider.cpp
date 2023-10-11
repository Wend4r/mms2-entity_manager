

#include "entitysystem_provider.h"

#include <entity_manager/placement.h>

extern EntityManagerSpace::Placement *g_pEntityManagerPlacement;

CEntityInstance *EntityManagerSpace::CEntitySystemProvider::CreateEntity(CEntityIndex iForcedIndex, const char *pszNameOrDesignName, EntityNetworkingMode_t eNetworkMode, SpawnGroupHandle_t hSpawnGroup, int iForcedSerial, bool bCreateInIsolatedPrecacheList)
{
	return g_pEntityManagerPlacement->m_aData.m_aEntitySystem.m_pfnCreateEntity(this, iForcedIndex, pszNameOrDesignName, eNetworkMode, hSpawnGroup, iForcedSerial, bCreateInIsolatedPrecacheList);
}
