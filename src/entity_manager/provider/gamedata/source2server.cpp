#include <entity_manager/provider.hpp>

EntityManager::Provider::GameDataStorage::Source2Server::Source2Server()
{
	{
		auto &aCallbacks = m_aAddressCallbacks;

		aCallbacks.Insert(m_aGameConfig.GetSymbol("&s_GameEventManager"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &aAddress)
		{
			m_ppGameEventManager = aAddress.RCast<decltype(m_ppGameEventManager)>();
		});

		m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}

	{
		auto &aCallbacks = m_aOffsetCallbacks;

		aCallbacks.Insert(m_aGameConfig.GetSymbol("CSource2Server::GetGameEventManager"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			m_nGetterGameEventManager = nOffset;
		});

		m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::Source2Server::Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return m_aGameConfig.Load(pRoot, pGameConfig, vecMessages);
}

void EntityManager::Provider::GameDataStorage::Source2Server::Reset()
{
	m_ppGameEventManager = nullptr;

	m_nGetterGameEventManager = -1;
}

CGameEventManager **EntityManager::Provider::GameDataStorage::Source2Server::GetGameEventManagerPtr() const
{
	return m_ppGameEventManager;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::Source2Server::GetGameEventManagerOffset() const
{
	return m_nGetterGameEventManager;
}
