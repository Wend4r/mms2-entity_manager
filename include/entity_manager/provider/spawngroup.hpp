#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_SPAWNGROUP_HPP_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_SPAWNGROUP_HPP_

#include <ientitymgr.hpp>
#include <spawngroupmgr.h>

namespace EntityManager
{
	class CSpawnGroupAccessor : public IEntityManager::ISpawnGroupAccessor
	{
	public:
		CSpawnGroupAccessor(CSpawnGroupMgrGameSystem *pSpawnGroupManager = NULL);

	public:
		void SetManager(CSpawnGroupMgrGameSystem *pNewInstance);

	public: // IEntityManager::IGameSpawnGroupMgrAccess
		CSpawnGroupMgrGameSystem *GetManager() override;

		CMapSpawnGroup *Get(SpawnGroupHandle_t h) override;
		CUtlMap<SpawnGroupHandle_t, CMapSpawnGroup *> *GetSpawnGroups() override;

	private:
		CSpawnGroupMgrGameSystem *m_pSpawnGroupManager;
	};

	class CLoadingSpawnGroupProvider : public CLoadingSpawnGroup
	{
	public:
		int AddSpawnInfos(int iCount, const EntitySpawnInfo_t *pEntities);
		int FindAndRemoveSpawnInfoByKeyValues(const CEntityKeyValues *pElement, CUtlVector<CEntityIdentity *> &vecRemovedIdentities);

	private:
		CUtlVector<EntitySpawnInfo_t> *GetSpawnInfo();
	};

	class CBaseSpawnGroupProvider : public ISpawnGroup
	{
	public:
		void SetEntityFilterName(const char *pNewValue);
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_SPAWNGROUP_HPP_
