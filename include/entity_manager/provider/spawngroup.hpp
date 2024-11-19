#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_SPAWNGROUP_HPP_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_SPAWNGROUP_HPP_

#include <functional>
#include <gamesystems/spawngroup_manager.h>

namespace EntityManager
{
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
