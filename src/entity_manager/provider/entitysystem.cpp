#include "entitykeyvalues.h"

#include <entity_manager/provider.h>

extern EntityManager::Provider *g_pEntityManagerProvider;

CEntityInstance *EntityManager::CEntitySystemProvider::CreateEntity(SpawnGroupHandle_t hSpawnGroup, const char *pszNameOrDesignName, EntityNetworkingMode_t eNetworkMode, CEntityIndex iForcedIndex, int iForcedSerial, bool bCreateInIsolatedPrecacheList)
{
	return g_pEntityManagerProvider->m_aData.m_aEntitySystem.m_pfnCreateEntity(this, hSpawnGroup, pszNameOrDesignName, eNetworkMode, iForcedIndex, iForcedSerial, bCreateInIsolatedPrecacheList);
}

void EntityManager::CEntitySystemProvider::QueueSpawnEntity(CEntityIdentity *pEntity, const CEntityKeyValues *pInitializationData)
{
	g_pEntityManagerProvider->m_aData.m_aEntitySystem.m_pfnQueueSpawnEntity(this, pEntity, pInitializationData);
}

void EntityManager::CEntitySystemProvider::ExecuteQueuedCreation()
{
	g_pEntityManagerProvider->m_aData.m_aEntitySystem.m_pfnExecuteQueuedCreation(this);
}
