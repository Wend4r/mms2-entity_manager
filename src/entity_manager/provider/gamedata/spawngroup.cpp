#include <entity_manager/provider_agent.hpp>
#include <entity_manager/provider.hpp>

DLL_IMPORT EntityManager::ProviderAgent *g_pEntityManagerProviderAgent;

EntityManager::Provider::GameDataStorage::SpawnGroup::SpawnGroup()
{
	{
		auto &aCallbacks = this->m_aAddressCallbacks;

		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("&g_pSpawnGroupMgr"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &pFunction)
		{
			this->m_ppSpawnGroupMgrAddress = pFunction.RCast<decltype(this->m_ppSpawnGroupMgrAddress)>();
			// g_pEntityManagerProviderAgent->NotifySpawnGroupMgrUpdated();
		});

		this->m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}
	
	{
		auto &aCallbacks = this->m_aOffsetCallbacks;

		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CSpawnGroupMgrGameSystem::m_SpawnGroups"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			this->m_nMgrGameSystemSpawnGroupsOffset = nOffset;
		});
		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CLoadingMapGroup::m_spawnInfo"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			this->m_nLoadingMapSpawnInfoOffset = nOffset;
		});
		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CBaseSpawnGroup::m_pEntityFilterName"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			this->m_nBaseSpawnGroupEntityFilterNameOffset = nOffset;
		});

		this->m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::SpawnGroup::Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return this->m_aGameConfig.Load(pRoot, pGameConfig, vecMessages);
}

void EntityManager::Provider::GameDataStorage::SpawnGroup::Reset()
{
	this->m_aGameConfig.ClearValues();

	this->m_ppSpawnGroupMgrAddress = nullptr;

	this->m_nMgrGameSystemSpawnGroupsOffset = -1;
	this->m_nLoadingMapSpawnInfoOffset = -1;
	this->m_nBaseSpawnGroupEntityFilterNameOffset = -1;
}

CSpawnGroupMgrGameSystem **EntityManager::Provider::GameDataStorage::SpawnGroup::GetSpawnGroupMgrAddress() const
{
	return this->m_ppSpawnGroupMgrAddress;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::SpawnGroup::GetMgrGameSystemSpawnGroupsOffset() const
{
	return this->m_nMgrGameSystemSpawnGroupsOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::SpawnGroup::GetLoadingMapSpawnInfoOffset() const
{
	return this->m_nLoadingMapSpawnInfoOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::SpawnGroup::GetBaseSpawnGroupEntityFilterNameOffset() const
{
	return this->m_nBaseSpawnGroupEntityFilterNameOffset;
}
