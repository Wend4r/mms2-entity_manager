#include <entity_manager/provider.hpp>

EntityManager::Provider::GameDataStorage::GameSystem::GameSystem()
{
	{
		auto &aCallbacks = m_aAddressCallbacks;

		aCallbacks.Insert(m_aGameConfig.GetSymbol("CBaseGameSystemFactory::sm_pFirst"), {[this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &aAddress)
		{
			m_pBaseGameSystemFactoryFirst = aAddress.RCast<decltype(m_pBaseGameSystemFactoryFirst)>();
		}});

		aCallbacks.Insert(m_aGameConfig.GetSymbol("IGameSystem::InitAllSystems"), {[this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &aAddress)
		{
			m_pfnGameSystemInit = aAddress.UCast<decltype(m_pfnGameSystemInit)>();
		}});

		m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::GameSystem::Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages)
{
	return m_aGameConfig.Load(pRoot, pGameConfig, vecMessages);
}

void EntityManager::Provider::GameDataStorage::GameSystem::Reset()
{
	m_pBaseGameSystemFactoryFirst = nullptr;
	m_pfnGameSystemInit = nullptr;
}

CBaseGameSystemFactory **EntityManager::Provider::GameDataStorage::GameSystem::GetBaseGameSystemFactoryFirst() const
{
	return m_pBaseGameSystemFactoryFirst;
}

EntityManager::Provider::GameDataStorage::GameSystem::OnGameSystemInitPtr EntityManager::Provider::GameDataStorage::GameSystem::GameSystemInitFunction() const
{
	return m_pfnGameSystemInit;
}
