#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_RESOURCEMANIFEST_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_RESOURCEMANIFEST_

#include <gamesystems/spawngroup_manager.h>

namespace EntityManager
{
	class ResourceManifest
	{
	public:
		~ResourceManifest();

		bool Init(ResourceManifestLoadBehavior_t eBehavior = RESOURCE_MANIFEST_LOAD_DEFAULT, const char *pszAllocatorName = "EntityManager::ResourceManifest_ctor", ResourceManifestLoadPriority_t ePriority = RESOURCE_MANIFEST_LOAD_PRIORITY_DEFAULT);
		bool Reinit(ResourceManifestLoadBehavior_t eBehavior = RESOURCE_MANIFEST_LOAD_DEFAULT, const char *pszAllocatorName = "EntityManager::ResourceManifest_ctor", ResourceManifestLoadPriority_t ePriority = RESOURCE_MANIFEST_LOAD_PRIORITY_DEFAULT);
		bool Destroy();

		bool Erect(SpawnGroupHandle_t hSpawnGroup, int nCount, const EntitySpawnInfo_t *pEntities, const matrix3x4a_t *const vWorldOffset);
		IEntityResourceManifest *GetEntityPart();

	private:
		CGameResourceManifest *m_pStorage;
	};
};

#endif // _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_RESOURCEMANIFEST_
