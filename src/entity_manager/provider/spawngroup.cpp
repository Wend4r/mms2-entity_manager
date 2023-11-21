#include "spawngroup.h"
#include "provider.h"

#include <tier1/utlmap.h>

extern EntityManager::Provider *g_pEntityManagerProvider;

CMapSpawnGroup *EntityManager::CSpawnGroupMgrGameSystemProvider::Get(SpawnGroupHandle_t h)
{
	auto pMgrSpawnGroupMap = this->GetSpawnGroups();

	auto iFoundIndex = pMgrSpawnGroupMap->Find(h);

	return iFoundIndex == pMgrSpawnGroupMap->InvalidIndex() ? nullptr : pMgrSpawnGroupMap->Element(iFoundIndex);
}

void EntityManager::CSpawnGroupMgrGameSystemProvider::LoopBySpawnGroups(std::function<EachSpawnGroupFunc> funcEachFunc)
{
	auto pMgrSpawnGroupMap = this->GetSpawnGroups();

	{
		const int iInvalidIndex = pMgrSpawnGroupMap->InvalidIndex();

		for(int i = pMgrSpawnGroupMap->FirstInorder(); i != iInvalidIndex; i = pMgrSpawnGroupMap->NextInorder(i))
		{
			funcEachFunc(pMgrSpawnGroupMap->Key(i), pMgrSpawnGroupMap->Element(i));
		}
	}
}

void EntityManager::CSpawnGroupMgrGameSystemProvider::FastLoopBySpawnGroups(std::function<EachSpawnGroupFunc> funcEachFunc)
{
	auto pMgrSpawnGroupMap = this->GetSpawnGroups();

	{
		const int iMaxElement = pMgrSpawnGroupMap->MaxElement();

		for(int i = 0; i < iMaxElement; ++i)
		{
			if(pMgrSpawnGroupMap->IsValidIndex(i))
			{
				funcEachFunc(pMgrSpawnGroupMap->Key(i), pMgrSpawnGroupMap->Element(i));
			}
		}
	}
}

CUtlMap<SpawnGroupHandle_t, CMapSpawnGroup *> *EntityManager::CSpawnGroupMgrGameSystemProvider::GetSpawnGroups()
{
	return (CUtlMap<SpawnGroupHandle_t, CMapSpawnGroup *> *)((uintptr_t)this + g_pEntityManagerProvider->GetGameDataStorage().GetSpawnGroup().GetMgrGameSystemSpawnGroupsOffset());
}

int EntityManager::CLoadingSpawnGroupProvider::AddSpawnInfos(int iCount, const EntitySpawnInfo_t *pEntities)
{
	return this->GetSpawnInfo()->AddMultipleToTail(iCount, pEntities);
}

int EntityManager::CLoadingSpawnGroupProvider::FindAndRemoveSpawnInfoByKeyValues(const CEntityKeyValues *pElement, CUtlVector<CEntityIdentity *> &vecRemovedIdentities)
{
	auto vecSpawnInfo = this->GetSpawnInfo();

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
