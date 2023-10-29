#include "entitykeyvalues.h"

#include <entity_manager/provider.h>

extern EntityManager::Provider *g_pEntityManagerProvider;

IEntityResourceManifest *EntityManager::CEntitySystemProvider::GetCurrentManifest()
{
	return *(IEntityResourceManifest **)((uintptr_t)this + g_pEntityManagerProvider->GetGameDataStorage().GetEntitySystem().GetCurrentManifestOffset());
}

CUtlScratchMemoryPool *EntityManager::CEntitySystemProvider::GetKeyValuesMemoryPool()
{
	return (CUtlScratchMemoryPool *)((uintptr_t)this + g_pEntityManagerProvider->GetGameDataStorage().GetEntitySystem().GetKeyValuesMemoryPoolOffset());
}

CEntityInstance *EntityManager::CEntitySystemProvider::CreateEntity(SpawnGroupHandle_t hSpawnGroup, const char *pszNameOrDesignName, EntityNetworkingMode_t eNetworkMode, CEntityIndex iForcedIndex, int iForcedSerial, bool bCreateInIsolatedPrecacheList)
{
	return g_pEntityManagerProvider->GetGameDataStorage().GetEntitySystem().CreateEntityFunction()(this, hSpawnGroup, pszNameOrDesignName, eNetworkMode, iForcedIndex, iForcedSerial, bCreateInIsolatedPrecacheList);
}

void EntityManager::CEntitySystemProvider::QueueSpawnEntity(CEntityIdentity *pEntity, const CEntityKeyValues *pInitializationData)
{
	g_pEntityManagerProvider->GetGameDataStorage().GetEntitySystem().QueueSpawnEntityFunction()(this, pEntity, pInitializationData);
}

void EntityManager::CEntitySystemProvider::ExecuteQueuedCreation()
{
	g_pEntityManagerProvider->GetGameDataStorage().GetEntitySystem().ExecuteQueuedCreationFunction()(this);
}

void EntityManager::CEntitySystemProvider::QueueDestroyEntity(CEntityIdentity *pEntity)
{
	g_pEntityManagerProvider->GetGameDataStorage().GetEntitySystem().QueueDestroyEntityFunction()(this, pEntity);
}

void EntityManager::CEntitySystemProvider::ExecuteQueuedDeletion(bool bPerformDeallocation)
{
	g_pEntityManagerProvider->GetGameDataStorage().GetEntitySystem().ExecuteQueuedDeletionFunction()(this, bPerformDeallocation);
}
