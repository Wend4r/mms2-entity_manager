#include <entity_manager/provider.hpp>

EntityManager::Provider::GameDataStorage::EntitySystem::EntitySystem()
{
	{
		auto &aCallbacks = this->m_aAddressCallbacks;

		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CEntitySystem::CreateEntity"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &pFunction)
		{
			this->m_pfnCreateEntity = pFunction.RCast<decltype(this->m_pfnCreateEntity)>();
		});
		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CEntitySystem::QueueSpawnEntity"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &pFunction)
		{
			this->m_pfnQueueSpawnEntity = pFunction.RCast<decltype(this->m_pfnQueueSpawnEntity)>();
		});
		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CEntitySystem::QueueDestroyEntity"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &pFunction)
		{
			this->m_pfnQueueDestroyEntity = pFunction.RCast<decltype(this->m_pfnQueueDestroyEntity)>();
		});
		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CEntitySystem::ExecuteQueuedCreation"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &pFunction)
		{
			this->m_pfnExecuteQueuedCreation = pFunction.RCast<decltype(this->m_pfnExecuteQueuedCreation)>();
		});
		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CEntitySystem::ExecuteQueuedDeletion"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &pFunction)
		{
			this->m_pfnExecuteQueuedDeletion = pFunction.RCast<decltype(this->m_pfnExecuteQueuedDeletion)>();
		});
		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CGameEntitySystem::ListenForEntityInSpawnGroupToFinish"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &pFunction)
		{
			this->m_pfnListenForEntityInSpawnGroupToFinish = pFunction.RCast<decltype(this->m_pfnListenForEntityInSpawnGroupToFinish)>();
		});

		this->m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}
	
	{
		auto &aCallbacks = this->m_aOffsetCallbacks;

		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CEntitySystem::m_pCurrentManifest"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			this->m_nCurrentManifestOffset = nOffset;
		});
		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CEntitySystem::m_aEntityKeyValuesAllocator"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			this->m_nEntityKeyValuesAllocatorOffset = nOffset;
		});

		this->m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::EntitySystem::Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return this->m_aGameConfig.Load(pRoot, pGameConfig, vecMessages);
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
