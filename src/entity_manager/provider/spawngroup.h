#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_SPAWNGROUP_PROVIDER_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_SPAWNGROUP_PROVIDER_H_

#include <functional>
#include <game_systems/spawngroup_manager.h>

namespace EntityManager
{
	class CSpawnGroupMgrGameSystemProvider : public CSpawnGroupMgrGameSystem
	{
	public:
		CMapSpawnGroup *Get(SpawnGroupHandle_t h);

	public: // Custom methods.
		typedef void (EachSpawnGroupFunc)(SpawnGroupHandle_t h, CMapSpawnGroup *pSpawnGroup); // Return true to continue, otherwise break.
		typedef EachSpawnGroupFunc *EachSpawnGroupFuncPtr;
		void WhileBySpawnGroups(std::function<EachSpawnGroupFunc> funcEachFunc); // In order.
		void FastWhileBySpawnGroups(std::function<EachSpawnGroupFunc> funcEachFunc);

	private:
		CUtlMap<SpawnGroupHandle_t, CMapSpawnGroup *> *GetSpawnGroups();
	};

	class CLoadingMapGroupProvider : public CLoadingSpawnGroup
	{
	public:
		int AddSpawnInfos(const EntitySpawnInfo_t *pEntities, int iCount);
		int FindAndRemoveSpawnInfoByKeyValues(const CEntityKeyValues *pElement, CUtlVector<CEntityIdentity *> &vecRemovedIdentities);

	private:
		CUtlVector<EntitySpawnInfo_t> *GetSpawnInfo();
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_SPAWNGROUP_PROVIDER_H_
