#include "provider.h"

EntityManager::Provider::GameDataStorage::EntitySystem::EntitySystem()
{
	{
		auto &aCallbacks = this->m_aAddressCallbacks;

		aCallbacks.Insert("CEntitySystem::CreateEntity", [this](const std::string &, const DynLibUtils::CMemory &pFucntion)
		{
			this->m_pfnCreateEntity = pFucntion.RCast<decltype(this->m_pfnCreateEntity)>();
		});
		aCallbacks.Insert("CEntitySystem::QueueSpawnEntity", [this](const std::string &, const DynLibUtils::CMemory &pFucntion)
		{
			this->m_pfnQueueSpawnEntity = pFucntion.RCast<decltype(this->m_pfnQueueSpawnEntity)>();
		});
		aCallbacks.Insert("CEntitySystem::QueueDestroyEntity", [this](const std::string &, const DynLibUtils::CMemory &pFucntion)
		{
			this->m_pfnQueueDestroyEntity = pFucntion.RCast<decltype(this->m_pfnQueueDestroyEntity)>();
		});
		aCallbacks.Insert("CEntitySystem::ExecuteQueuedCreation", [this](const std::string &, const DynLibUtils::CMemory &pFucntion)
		{
			this->m_pfnExecuteQueuedCreation = pFucntion.RCast<decltype(this->m_pfnExecuteQueuedCreation)>();
		});
		aCallbacks.Insert("CEntitySystem::ExecuteQueuedDeletion", [this](const std::string &, const DynLibUtils::CMemory &pFucntion)
		{
			this->m_pfnExecuteQueuedDeletion = pFucntion.RCast<decltype(this->m_pfnExecuteQueuedDeletion)>();
		});
		aCallbacks.Insert("CGameEntitySystem::ListenForEntityInSpawnGroupToFinish", [this](const std::string &, const DynLibUtils::CMemory &pFucntion)
		{
			this->m_pfnListenForEntityInSpawnGroupToFinish = pFucntion.RCast<decltype(this->m_pfnListenForEntityInSpawnGroupToFinish)>();
		});

		this->m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}
	
	{
		auto &aCallbacks = this->m_aOffsetCallbacks;

		aCallbacks.Insert("CEntitySystem::m_pCurrentManifest", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nCurrentManifestOffset = nOffset;
		});
		aCallbacks.Insert("CEntitySystem::m_aEntityKeyValuesAllocator", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nEntityKeyValuesAllocatorOffset = nOffset;
		});

		this->m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::EntitySystem::Load(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aGameConfig.Load(pRoot, pGameConfig, psError, nMaxLength);
}

void EntityManager::Provider::GameDataStorage::EntitySystem::Reset()
{
	this->m_aGameConfig.ClearValues();

	this->m_pfnCreateEntity = nullptr;
	this->m_pfnQueueSpawnEntity = nullptr;
	this->m_pfnExecuteQueuedCreation = nullptr;

	this->m_nCurrentManifestOffset = -1;
	this->m_nEntityKeyValuesAllocatorOffset = -1;
}

EntityManager::Provider::GameDataStorage::EntitySystem::OnCreateEntityPtr EntityManager::Provider::GameDataStorage::EntitySystem::CreateEntityFunction() const
{
	return this->m_pfnCreateEntity;
}

EntityManager::Provider::GameDataStorage::EntitySystem::OnQueueSpawnEntityPtr EntityManager::Provider::GameDataStorage::EntitySystem::QueueSpawnEntityFunction() const
{
	return this->m_pfnQueueSpawnEntity;
}

EntityManager::Provider::GameDataStorage::EntitySystem::OnQueueDestroyEntityPtr EntityManager::Provider::GameDataStorage::EntitySystem::QueueDestroyEntityFunction() const
{
	return this->m_pfnQueueDestroyEntity;
}

EntityManager::Provider::GameDataStorage::EntitySystem::OnExecuteQueuedCreationPtr EntityManager::Provider::GameDataStorage::EntitySystem::ExecuteQueuedCreationFunction() const
{
	return this->m_pfnExecuteQueuedCreation;
}

EntityManager::Provider::GameDataStorage::EntitySystem::OnExecuteQueuedDeletionPtr EntityManager::Provider::GameDataStorage::EntitySystem::ExecuteQueuedDeletionFunction() const
{
	return this->m_pfnExecuteQueuedDeletion;
}

EntityManager::Provider::GameDataStorage::EntitySystem::OnListenForEntityInSpawnGroupToFinishPtr EntityManager::Provider::GameDataStorage::EntitySystem::ListenForEntityInSpawnGroupToFinishFunction() const
{
	return this->m_pfnListenForEntityInSpawnGroupToFinish;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::EntitySystem::GetCurrentManifestOffset() const
{
	return this->m_nCurrentManifestOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::EntitySystem::GetKeyValuesContextAllocatorOffset() const
{
	return this->m_nEntityKeyValuesAllocatorOffset;
}
