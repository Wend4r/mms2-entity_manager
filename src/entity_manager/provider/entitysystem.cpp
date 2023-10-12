#include "entitykeyvalues.h"

#include <entity_manager/provider.h>

extern EntityManagerSpace::Provider *g_pEntityManagerProvider;

CEntityInstance *EntityManagerSpace::CEntitySystemProvider::CreateEntity(CEntityIndex iForcedIndex, const char *pszNameOrDesignName, EntityNetworkingMode_t eNetworkMode, SpawnGroupHandle_t hSpawnGroup, int iForcedSerial, bool bCreateInIsolatedPrecacheList)
{
	return g_pEntityManagerProvider->m_aData.m_aEntitySystem.m_pfnCreateEntity(this, iForcedIndex, pszNameOrDesignName, eNetworkMode, hSpawnGroup, iForcedSerial, bCreateInIsolatedPrecacheList);
}

void EntityManagerSpace::CEntitySystemProvider::QueueSpawnEntity(CEntityIdentity *pEntity, const CEntityKeyValues *pInitializationData)
{
	g_pEntityManagerProvider->m_aData.m_aEntitySystem.m_pfnQueueSpawnEntity(this, pEntity, pInitializationData);
}

void EntityManagerSpace::CEntitySystemProvider::ExecuteQueuedCreation()
{
	g_pEntityManagerProvider->m_aData.m_aEntitySystem.m_pfnExecuteQueuedCreation(this);
}
