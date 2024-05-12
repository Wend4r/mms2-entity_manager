#include <entity2/entitykeyvalues.h>

#include "entitysystem.hpp"
#include "provider.hpp"

DLL_IMPORT EntityManager::Provider *g_pEntityManagerProvider;

IEntityResourceManifest *EntityManager::CEntitySystemProvider::GetCurrentManifest()
{
	return *(IEntityResourceManifest **)((uintptr_t)this + g_pEntityManagerProvider->GetGameDataStorage().GetEntitySystem().GetCurrentManifestOffset());
}

CKeyValues3Context *EntityManager::CEntitySystemProvider::GetKeyValuesContextAllocator()
{
	return (CKeyValues3Context *)((uintptr_t)this + g_pEntityManagerProvider->GetGameDataStorage().GetEntitySystem().GetKeyValuesContextAllocatorOffset());
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

void EntityManager::CEntitySystemProvider::ListenForEntityInSpawnGroupToFinish(SpawnGroupHandle_t hSpawnGroup, CEntityInstance *pEntityToListenFor, const CEntityKeyValues *pKeyValues, CEntityInstance *pListener, CUtlDelegate<void (CEntityInstance *, const CEntityKeyValues *)> handler)
{
	g_pEntityManagerProvider->GetGameDataStorage().GetEntitySystem().ListenForEntityInSpawnGroupToFinishFunction()(this, hSpawnGroup, pEntityToListenFor, pKeyValues, pListener, handler);
}
