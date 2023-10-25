#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYSYSTEM_PROVIDER_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYSYSTEM_PROVIDER_H_

#include <eiface.h>
#include <entity2/entityidentity.h>
#include <entity2/entitysystem.h>

enum EntityNetworkingMode_t
{
	ENTITY_NETWORKING_MODE_DEFAULT = 0,
	ENTITY_NETWORKING_MODE_NETWORKED,
	ENTITY_NETWORKING_MODE_NOT_NETWORKED,
};

namespace EntityManager
{
	class CEntitySystemProvider : public CEntitySystem
	{
	public:
		IEntityResourceManifest *GetCurrentManifest();
		CUtlScratchMemoryPool *GetKeyValuesMemoryPool();

	public: // Action methods.
		CEntityInstance *CreateEntity(SpawnGroupHandle_t hSpawnGroup, const char *pszNameOrDesignName, EntityNetworkingMode_t eNetworkMode, CEntityIndex iForcedIndex, int iForcedSerial, bool bCreateInIsolatedPrecacheList);
		void QueueSpawnEntity(CEntityIdentity *pEntity, const CEntityKeyValues *pInitializationData);
		void ExecuteQueuedCreation();
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYSYSTEM_PROVIDER_H_
