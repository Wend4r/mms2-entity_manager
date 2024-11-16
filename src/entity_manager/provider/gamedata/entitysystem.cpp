#include <entity_manager/provider.hpp>

EntityManager::Provider::GameDataStorage::EntitySystem::EntitySystem()
{
	{
		auto &aCallbacks = m_aAddressCallbacks;

		aCallbacks.Insert(m_aGameConfig.GetSymbol("CEntitySystem::CreateEntity"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &pFunction)
		{
			m_pfnCreateEntity = pFunction.RCast<decltype(m_pfnCreateEntity)>();
		});
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CEntitySystem::QueueSpawnEntity"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &pFunction)
		{
			m_pfnQueueSpawnEntity = pFunction.RCast<decltype(m_pfnQueueSpawnEntity)>();
		});
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CEntitySystem::QueueDestroyEntity"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &pFunction)
		{
			m_pfnQueueDestroyEntity = pFunction.RCast<decltype(m_pfnQueueDestroyEntity)>();
		});
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CEntitySystem::ExecuteQueuedCreation"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &pFunction)
		{
			m_pfnExecuteQueuedCreation = pFunction.RCast<decltype(m_pfnExecuteQueuedCreation)>();
		});
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CEntitySystem::ExecuteQueuedDeletion"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &pFunction)
		{
			m_pfnExecuteQueuedDeletion = pFunction.RCast<decltype(m_pfnExecuteQueuedDeletion)>();
		});
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CGameEntitySystem::ListenForEntityInSpawnGroupToFinish"), [this](const CUtlSymbolLarge &, const DynLibUtils::CMemory &pFunction)
		{
			m_pfnListenForEntityInSpawnGroupToFinish = pFunction.RCast<decltype(m_pfnListenForEntityInSpawnGroupToFinish)>();
		});

		m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}
	
	{
		auto &aCallbacks = m_aOffsetCallbacks;

		aCallbacks.Insert(m_aGameConfig.GetSymbol("CEntitySystem::m_pCurrentManifest"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			m_nCurrentManifestOffset = nOffset;
		});
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CEntitySystem::m_aEntityKeyValuesAllocator"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			m_nEntityKeyValuesAllocatorOffset = nOffset;
		});

		m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::EntitySystem::Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return m_aGameConfig.Load(pRoot, pGameConfig, vecMessages);
}

void EntityManager::Provider::GameDataStorage::EntitySystem::Reset()
{
	m_aGameConfig.ClearValues();

	m_pfnCreateEntity = nullptr;
	m_pfnQueueSpawnEntity = nullptr;
	m_pfnExecuteQueuedCreation = nullptr;

	m_nCurrentManifestOffset = -1;
	m_nEntityKeyValuesAllocatorOffset = -1;
}

EntityManager::Provider::GameDataStorage::EntitySystem::OnCreateEntityPtr EntityManager::Provider::GameDataStorage::EntitySystem::CreateEntityFunction() const
{
	return m_pfnCreateEntity;
}

EntityManager::Provider::GameDataStorage::EntitySystem::OnQueueSpawnEntityPtr EntityManager::Provider::GameDataStorage::EntitySystem::QueueSpawnEntityFunction() const
{
	return m_pfnQueueSpawnEntity;
}

EntityManager::Provider::GameDataStorage::EntitySystem::OnQueueDestroyEntityPtr EntityManager::Provider::GameDataStorage::EntitySystem::QueueDestroyEntityFunction() const
{
	return m_pfnQueueDestroyEntity;
}

EntityManager::Provider::GameDataStorage::EntitySystem::OnExecuteQueuedCreationPtr EntityManager::Provider::GameDataStorage::EntitySystem::ExecuteQueuedCreationFunction() const
{
	return m_pfnExecuteQueuedCreation;
}

EntityManager::Provider::GameDataStorage::EntitySystem::OnExecuteQueuedDeletionPtr EntityManager::Provider::GameDataStorage::EntitySystem::ExecuteQueuedDeletionFunction() const
{
	return m_pfnExecuteQueuedDeletion;
}

EntityManager::Provider::GameDataStorage::EntitySystem::OnListenForEntityInSpawnGroupToFinishPtr EntityManager::Provider::GameDataStorage::EntitySystem::ListenForEntityInSpawnGroupToFinishFunction() const
{
	return m_pfnListenForEntityInSpawnGroupToFinish;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::EntitySystem::GetCurrentManifestOffset() const
{
	return m_nCurrentManifestOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::EntitySystem::GetKeyValuesContextAllocatorOffset() const
{
	return m_nEntityKeyValuesAllocatorOffset;
}
