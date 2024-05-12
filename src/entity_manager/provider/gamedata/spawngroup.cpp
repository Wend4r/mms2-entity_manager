#include "provider.h"
#include "provider_agent.h"

DLL_IMPORT EntityManager::ProviderAgent *g_pEntityManagerProviderAgent;

EntityManager::Provider::GameDataStorage::SpawnGroup::SpawnGroup()
{
	{
		auto &aCallbacks = this->m_aAddressCallbacks;

		aCallbacks.Insert("&g_pSpawnGroupMgr", [this](const std::string &, const DynLibUtils::CMemory &pFunction)
		{
			this->m_ppSpawnGroupMgrAddress = pFunction.RCast<decltype(this->m_ppSpawnGroupMgrAddress)>();
			g_pEntityManagerProviderAgent->NotifySpawnGroupMgrUpdated();
		});

		this->m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}
	
	{
		auto &aCallbacks = this->m_aOffsetCallbacks;

		aCallbacks.Insert("CSpawnGroupMgrGameSystem::m_SpawnGroups", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nMgrGameSystemSpawnGroupsOffset = nOffset;
		});
		aCallbacks.Insert("CLoadingMapGroup::m_spawnInfo", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nLoadingMapSpawnInfoOffset = nOffset;
		});
		aCallbacks.Insert("CBaseSpawnGroup::m_pEntityFilterName", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nBaseSpawnGroupEntityFilterNameOffset = nOffset;
		});

		this->m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::SpawnGroup::Load(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aGameConfig.Load(pRoot, pGameConfig, psError, nMaxLength);
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
