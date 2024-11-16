#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_SPAWNGROUP_HPP_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_SPAWNGROUP_HPP_

#include <functional>
#include <gamesystems/spawngroup_manager.h>

namespace EntityManager
{
	class CSpawnGroupMgrGameSystemProvider : public CSpawnGroupMgrGameSystem
	{
	public:
		CMapSpawnGroup *Get(SpawnGroupHandle_t h);

	public: // Custom methods.
		typedef void (EachSpawnGroupFunc)(SpawnGroupHandle_t h, CMapSpawnGroup *pSpawnGroup); // Return true to continue, otherwise break.
		typedef EachSpawnGroupFunc *EachSpawnGroupFuncPtr;
		void LoopBySpawnGroups(std::function<EachSpawnGroupFunc> funcEachFunc); // In order.
		void FastLoopBySpawnGroups(std::function<EachSpawnGroupFunc> funcEachFunc);

	private:
		CUtlMap<SpawnGroupHandle_t, CMapSpawnGroup *> *GetSpawnGroups();
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
