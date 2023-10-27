#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_GAMERESOURCE_PROVIDER_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_GAMERESOURCE_PROVIDER_H_

#include <eiface.h>
#include <entity2/entityidentity.h>
#include <entity2/entitysystem.h>

class matrix3x4a_t;

class IGameResourceService
{
};

class CGameResourceService : public IGameResourceService
{
};

namespace EntityManager
{
	class CGameResourceServiceProvider : public CGameResourceService
	{
	public:
		void PrecacheEntitiesAndConfirmResourcesAreLoaded(SpawnGroupHandle_t hSpawnGroup, int nCount, const EntitySpawnInfo_t *pEntities, const matrix3x4a_t *const vWorldOffset);
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_GAMERESOURCE_PROVIDER_H_
