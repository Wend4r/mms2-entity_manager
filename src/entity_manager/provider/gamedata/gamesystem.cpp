#include <entity_manager/provider.hpp>

EntityManager::Provider::GameDataStorage::GameSystem::GameSystem()
{
	{
		auto &aCallbacks = this->m_aAddressCallbacks;

		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CBaseGameSystemFactory::sm_pFirst"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &aAddress)
		{
			this->m_pBaseGameSystemFactoryFirst = aAddress.RCast<decltype(this->m_pBaseGameSystemFactoryFirst)>();
		});

		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("IGameSystem::InitAllSystems"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &aAddress)
		{
			this->m_pfnGameSystemInit = aAddress.UCast<decltype(this->m_pfnGameSystemInit)>();
		});

		this->m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::GameSystem::Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return this->m_aGameConfig.Load(pRoot, pGameConfig, vecMessages);
}

void EntityManager::Provider::GameDataStorage::GameSystem::Reset()
{
	this->m_pBaseGameSystemFactoryFirst = nullptr;
	this->m_pfnGameSystemInit = nullptr;
}

CBaseGameSystemFactory **EntityManager::Provider::GameDataStorage::GameSystem::GetBaseGameSystemFactoryFirst() const
{
	return this->m_pBaseGameSystemFactoryFirst;
}

EntityManager::Provider::GameDataStorage::GameSystem::OnGameSystemInitPtr EntityManager::Provider::GameDataStorage::GameSystem::GameSystemInitFunction() const
{
	return this->m_pfnGameSystemInit;
}
