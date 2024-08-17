#include <entity_manager/provider.hpp>

EntityManager::Provider::GameDataStorage::Source2Server::Source2Server()
{
	{
		auto &aCallbacks = this->m_aAddressCallbacks;

		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("&s_GameEventManager"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &aAddress)
		{
			this->m_ppGameEventManager = aAddress.RCast<decltype(this->m_ppGameEventManager)>();
		});

		this->m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}

	{
		auto &aCallbacks = this->m_aOffsetCallbacks;

		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CSource2Server::GetGameEventManager"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			this->m_nGetterGameEventManager = nOffset;
		});

		this->m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::Source2Server::Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return this->m_aGameConfig.Load(pRoot, pGameConfig, vecMessages);
}

void EntityManager::Provider::GameDataStorage::Source2Server::Reset()
{
	this->m_ppGameEventManager = nullptr;

	this->m_nGetterGameEventManager = -1;
}

CGameEventManager **EntityManager::Provider::GameDataStorage::Source2Server::GetGameEventManagerPtr() const
{
	return this->m_ppGameEventManager;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::Source2Server::GetGameEventManagerOffset() const
{
	return this->m_nGetterGameEventManager;
}
