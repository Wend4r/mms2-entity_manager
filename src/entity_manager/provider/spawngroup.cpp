#include "spawngroup.h"

#include <tier1/utlmap.h>
#include <entity_manager/provider.h>

extern EntityManager::Provider *g_pEntityManagerProvider;

CMapSpawnGroup *EntityManager::CSpawnGroupMgrGameSystemProvider::Get(SpawnGroupHandle_t h)
{
	auto pMgrSpawnGroupMap = this->GetSpawnGroups();

	auto iFoundIndex = pMgrSpawnGroupMap->Find(h);

	return iFoundIndex == pMgrSpawnGroupMap->InvalidIndex() ? nullptr : pMgrSpawnGroupMap->Element(iFoundIndex);
}

void EntityManager::CSpawnGroupMgrGameSystemProvider::WhileBySpawnGroups(std::function<EachSpawnGroupFunc> funcEachFunc)
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

void EntityManager::CSpawnGroupMgrGameSystemProvider::FastWhileBySpawnGroups(std::function<EachSpawnGroupFunc> funcEachFunc)
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
