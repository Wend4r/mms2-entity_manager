#include "gameresource.h"
#include "provider.h"

#include <memory_utils/virtual.h>

extern EntityManager::Provider *g_pEntityManagerProvider;

void EntityManager::CGameResourceServiceProvider::DestroyResourceManifest(CGameResourceManifest *pTarget)
{
	((VirtualTable *)this)->CallMethod<void>(g_pEntityManagerProvider->GetGameDataStorage().GetGameResource().GetDestroyResourceManifestOffset(), pTarget);
}

void EntityManager::CGameResourceServiceProvider::PrecacheEntitiesAndConfirmResourcesAreLoaded(SpawnGroupHandle_t hSpawnGroup, int nCount, const EntitySpawnInfo_t *pEntities, const matrix3x4a_t *const vWorldOffset)
{
	((VirtualTable *)this)->CallMethod<void>(g_pEntityManagerProvider->GetGameDataStorage().GetGameResource().GetPrecacheEntitiesAndConfirmResourcesAreLoadedOffset(), hSpawnGroup, nCount, pEntities, vWorldOffset);
}

CGameResourceManifest *EntityManager::CGameResourceServiceProvider::AllocGameResourceManifest(ResourceManifestLoadBehavior_t eBehavior, const char *pszAllocatorName, ResourceManifestLoadPriority_t ePriority)
{
	return ((VirtualTable *)this)->CallMethod<CGameResourceManifest *>(g_pEntityManagerProvider->GetGameDataStorage().GetGameResource().GetAllocGameResourceManifestOffset(), eBehavior, pszAllocatorName, ePriority);
}

bool EntityManager::CGameResourceServiceProvider::AppendToAndCreateGameResourceManifest(CGameResourceManifest *pResourceManifest, SpawnGroupHandle_t hSpawnGroup, int nCount, const EntitySpawnInfo_t *pEntities, const matrix3x4a_t * vWorldOffset)
{
	return ((VirtualTable *)this)->CallMethod<bool>(g_pEntityManagerProvider->GetGameDataStorage().GetGameResource().GetAppendToAndCreateGameResourceManifestOffset(), pResourceManifest, hSpawnGroup, nCount, pEntities, vWorldOffset);
}

CEntityResourceManifest *EntityManager::CGameResourceManifestProvider::GetEntityPart()
{
	return *(CEntityResourceManifest **)((uintptr_t)this + g_pEntityManagerProvider->GetGameDataStorage().GetGameResource().GetEntityManifestOffset());
}

IEntityResourceManifest *EntityManager::CEntityResourceManifestProvider::GetInterface()
{
	return *(IEntityResourceManifest **)((uintptr_t)this + g_pEntityManagerProvider->GetGameDataStorage().GetGameResource().GetEntityManifestVFTableOffset());
}
