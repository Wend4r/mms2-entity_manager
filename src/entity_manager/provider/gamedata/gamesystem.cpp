#include "provider.hpp"

EntityManager::Provider::GameDataStorage::GameSystem::GameSystem()
{
	{
		auto &aCallbacks = this->m_aAddressCallbacks;

		aCallbacks.Insert("CBaseGameSystemFactory::sm_pFirst", [this](const std::string &, const DynLibUtils::CMemory &aAddress)
		{
			this->m_pBaseGameSystemFactoryFirst = aAddress.RCast<decltype(this->m_pBaseGameSystemFactoryFirst)>();
		});

		aCallbacks.Insert("IGameSystem::InitAllSystems", [this](const std::string &, const DynLibUtils::CMemory &aAddress)
		{
			this->m_pfnGameSystemInit = aAddress.UCast<decltype(this->m_pfnGameSystemInit)>();
		});

		this->m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::GameSystem::Load(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aGameConfig.Load(pRoot, pGameConfig, psError, nMaxLength);
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
