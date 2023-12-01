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
	class CEntitySystemProvider : public CGameEntitySystem
	{
	public:
		IEntityResourceManifest *GetCurrentManifest();
		CKeyValues3Context *GetKeyValuesContextAllocator();

	public: // Action methods.
		CEntityInstance *CreateEntity(SpawnGroupHandle_t hSpawnGroup, const char *pszNameOrDesignName, EntityNetworkingMode_t eNetworkMode, CEntityIndex iForcedIndex, int iForcedSerial, bool bCreateInIsolatedPrecacheList);
		void QueueSpawnEntity(CEntityIdentity *pEntity, const CEntityKeyValues *pInitializationData);
		void ExecuteQueuedCreation();

		void QueueDestroyEntity(CEntityIdentity *pEntity);
		void ExecuteQueuedDeletion(bool bPerformDeallocation = false);

	public: // CGameEntitySystem.
		void ListenForEntityInSpawnGroupToFinish(SpawnGroupHandle_t hSpawnGroup, CEntityInstance *pEntityToListenFor, const CEntityKeyValues *pKeyValues, CEntityInstance *pListener, CUtlDelegate<void (CEntityInstance *, const CEntityKeyValues *)> handler);
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYSYSTEM_PROVIDER_H_
