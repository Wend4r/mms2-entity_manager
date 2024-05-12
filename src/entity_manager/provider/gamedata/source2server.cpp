#include "provider.hpp"

EntityManager::Provider::GameDataStorage::Source2Server::Source2Server()
{
	{
		auto &aCallbacks = this->m_aOffsetCallbacks;

		aCallbacks.Insert("CSource2Server::GetGameEventManager", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nGetterGameEventManager = nOffset;
		});

		this->m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::Source2Server::Load(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aGameConfig.Load(pRoot, pGameConfig, psError, nMaxLength);
}

void EntityManager::Provider::GameDataStorage::Source2Server::Reset()
{
	this->m_nGetterGameEventManager = -1;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::Source2Server::GetGameEventManagerOffset() const
{
	return this->m_nGetterGameEventManager;
}
