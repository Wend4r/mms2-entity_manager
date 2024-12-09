#include <entity_manager/provider/spawngroup.hpp>
#include <entity_manager/provider.hpp>

#include <tier1/utlmap.h>

DLL_IMPORT EntityManager::Provider *g_pEntityManagerProvider;

EntityManager::CSpawnGroupAccess::CSpawnGroupAccess(CSpawnGroupMgrGameSystem *pSpawnGroupManager)
 :  m_pSpawnGroupManager(pSpawnGroupManager)
{
}

CMapSpawnGroup *EntityManager::CSpawnGroupAccess::Get(SpawnGroupHandle_t h)
{
	auto *pMgrSpawnGroupMap = GetSpawnGroups();

	pMgrSpawnGroupMap->SetLessFunc(DefLessFunc(SpawnGroupHandle_t));

	auto iFoundIndex = pMgrSpawnGroupMap->Find(h);

	return iFoundIndex == pMgrSpawnGroupMap->InvalidIndex() ? nullptr : pMgrSpawnGroupMap->Element(iFoundIndex);
}

CUtlMap<SpawnGroupHandle_t, CMapSpawnGroup *> *EntityManager::CSpawnGroupAccess::GetSpawnGroups()
{
	return (CUtlMap<SpawnGroupHandle_t, CMapSpawnGroup *> *)((uintptr_t)m_pSpawnGroupManager + g_pEntityManagerProvider->GetGameDataStorage().GetSpawnGroup().GetMgrGameSystemSpawnGroupsOffset());
}

void EntityManager::CSpawnGroupAccess::SetManager(CSpawnGroupMgrGameSystem *pNewInstance)
{
	m_pSpawnGroupManager = pNewInstance;
}

CSpawnGroupMgrGameSystem *EntityManager::CSpawnGroupAccess::GetManager()
{
	return m_pSpawnGroupManager;
}

int EntityManager::CLoadingSpawnGroupProvider::AddSpawnInfos(int iCount, const EntitySpawnInfo_t *pEntities)
{
	return GetSpawnInfo()->AddMultipleToTail(iCount, pEntities);
}

int EntityManager::CLoadingSpawnGroupProvider::FindAndRemoveSpawnInfoByKeyValues(const CEntityKeyValues *pElement, CUtlVector<CEntityIdentity *> &vecRemovedIdentities)
{
	auto vecSpawnInfo = GetSpawnInfo();

	const int iSpawnInfoCount = vecSpawnInfo->Count();

	int iRemoveCount = 0;

	// Find KV element.
	for(int i = 0; i < iSpawnInfoCount; i++)
	{
		const auto &aInfo = vecSpawnInfo->Element(i);

		if(aInfo.m_pKeyValues == pElement)
		{
			vecRemovedIdentities.AddToTail(aInfo.m_pEntity);

			vecSpawnInfo->FastRemove(i);
			iRemoveCount++;
		}
	}

	return iRemoveCount;
}

CUtlVector<EntitySpawnInfo_t> *EntityManager::CLoadingSpawnGroupProvider::GetSpawnInfo()
{
	return (CUtlVector<EntitySpawnInfo_t> *)((uintptr_t)this + g_pEntityManagerProvider->GetGameDataStorage().GetSpawnGroup().GetLoadingMapSpawnInfoOffset());
}

void EntityManager::CBaseSpawnGroupProvider::SetEntityFilterName(const char *pNewValue)
{
	*(const char **)((uintptr_t)this + g_pEntityManagerProvider->GetGameDataStorage().GetSpawnGroup().GetBaseSpawnGroupEntityFilterNameOffset()) = pNewValue;
}
