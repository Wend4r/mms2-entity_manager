#include "gameresource.h"

#include <entity_manager/provider.h>
#include <memory_utils/virtual.h>

extern EntityManager::Provider *g_pEntityManagerProvider;

void EntityManager::CGameResourceServiceProvider::PrecacheEntitiesAndConfirmResourcesAreLoaded(SpawnGroupHandle_t hSpawnGroup, int nCount, const EntitySpawnInfo_t *pEntities, const matrix3x4a_t &vWorldOffset)
{
	((VirtualTable *)this)->CallMethod<void>(g_pEntityManagerProvider->GetGameDataStorage().GetGameResource().GetPrecacheEntitiesAndConfirmResourcesAreLoadedOffset(), hSpawnGroup, nCount, pEntities, vWorldOffset);
}
