#include <entity_manager/provider_agent.hpp>
#include <entity_manager/provider.hpp>

extern EntityManager::ProviderAgent *g_pEntityManagerProviderAgent;

EntityManager::Provider::GameDataStorage::SpawnGroup::SpawnGroup()
{
	{
		auto &aCallbacks = m_aAddressCallbacks;

		aCallbacks.Insert(m_aGameConfig.GetSymbol("&g_pSpawnGroupMgr"), {[this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &pFunction)
		{
			m_ppSpawnGroupMgrAddress = pFunction.RCast<decltype(m_ppSpawnGroupMgrAddress)>();
			g_pEntityManagerProviderAgent->NotifySpawnGroupMgrUpdated();
		}});

		m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}
	
	{
		auto &aCallbacks = m_aOffsetCallbacks;

		aCallbacks.Insert(m_aGameConfig.GetSymbol("CSpawnGroupMgrGameSystem::m_SpawnGroups"), {[this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			m_nMgrGameSystemSpawnGroupsOffset = nOffset;
		}});
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CLoadingMapGroup::m_spawnInfo"), {[this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			m_nLoadingMapSpawnInfoOffset = nOffset;
		}});
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CBaseSpawnGroup::m_pEntityFilterName"), {[this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			m_nBaseSpawnGroupEntityFilterNameOffset = nOffset;
		}});

		m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::SpawnGroup::Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return m_aGameConfig.Load(pRoot, pGameConfig, vecMessages);
}

void EntityManager::Provider::GameDataStorage::SpawnGroup::Reset()
{
	m_aGameConfig.ClearValues();

	m_ppSpawnGroupMgrAddress = nullptr;

	m_nMgrGameSystemSpawnGroupsOffset = -1;
	m_nLoadingMapSpawnInfoOffset = -1;
	m_nBaseSpawnGroupEntityFilterNameOffset = -1;
}

CSpawnGroupMgrGameSystem **EntityManager::Provider::GameDataStorage::SpawnGroup::GetSpawnGroupMgrAddress() const
{
	return m_ppSpawnGroupMgrAddress;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::SpawnGroup::GetMgrGameSystemSpawnGroupsOffset() const
{
	return m_nMgrGameSystemSpawnGroupsOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::SpawnGroup::GetLoadingMapSpawnInfoOffset() const
{
	return m_nLoadingMapSpawnInfoOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::SpawnGroup::GetBaseSpawnGroupEntityFilterNameOffset() const
{
	return m_nBaseSpawnGroupEntityFilterNameOffset;
}
