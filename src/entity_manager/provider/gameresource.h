#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_GAMERESOURCE_PROVIDER_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_GAMERESOURCE_PROVIDER_H_

#include <gamesystems/spawngroup_manager.h>

class IGameResourceService
{
};

class CGameResourceService : public IGameResourceService
{
};

class CGameResourceManifest
{
};

class CEntityResourceManifest
{
};

namespace EntityManager
{
	class CGameResourceServiceProvider : public CGameResourceService
	{
	public:
		void DestroyResourceManifest(CGameResourceManifest *pTarget);
		void PrecacheEntitiesAndConfirmResourcesAreLoaded(SpawnGroupHandle_t hSpawnGroup, int nCount, const EntitySpawnInfo_t *pEntities, const matrix3x4a_t *const vWorldOffset);
		CGameResourceManifest *AllocGameResourceManifest(ResourceManifestLoadBehavior_t eBehavior, const char *pszAllocatorName, ResourceManifestLoadPriority_t ePriority);
		bool AppendToAndCreateGameResourceManifest(CGameResourceManifest *pResourceManifest, SpawnGroupHandle_t hSpawnGroup, int nCount, const EntitySpawnInfo_t *pEntities, const matrix3x4a_t *const vWorldOffset);
	};

	class CGameResourceManifestProvider : public CGameResourceManifest
	{
	public:
		CEntityResourceManifest *GetEntityPart();
	};

	class CEntityResourceManifestProvider : public CEntityResourceManifest
	{
	public:
		IEntityResourceManifest *GetInterface();
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_GAMERESOURCE_PROVIDER_H_
