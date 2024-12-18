#include <entity_manager/provider_agent.hpp>
#include <entity_manager/provider.hpp>

DLL_IMPORT EntityManager::ProviderAgent *g_pEntityManagerProviderAgent;

EntityManager::Provider::GameDataStorage::GameResource::GameResource()
{
	{
		auto &aCallbacks = m_aOffsetCallbacks;

		aCallbacks.Insert(m_aGameConfig.GetSymbol("CGameResourceService::DestroyResourceManifest"), {[this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			m_nDestroyResourceManifestOffset = nOffset;
		}});
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CGameResourceService::PrecacheEntitiesAndConfirmResourcesAreLoaded"), {[this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			m_nPrecacheEntitiesAndConfirmResourcesAreLoadedOffset = nOffset;
		}});
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CGameResourceService::AllocGameResourceManifest"), {[this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			m_nAllocGameResourceManifestOffset = nOffset;
		}});
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CGameResourceService::AppendToAndCreateGameResourceManifest"), {[this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			m_nAppendToAndCreateGameResourceManifestOffset = nOffset;
		}});
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CGameResourceService::m_pEntitySystem"), {[this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			m_nEntitySystemOffset = nOffset;
			g_pEntityManagerProviderAgent->NotifyEntitySystemUpdated();
		}});
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CGameResourceService::m_pEntityManifest"), {[this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			m_nEntityManifestOffset = nOffset;
		}});
		aCallbacks.Insert(m_aGameConfig.GetSymbol("CEntityResourceManifest::`vftable'"), {[this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			m_nEntityManifestVFTableOffset = nOffset;
		}});

		m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::GameResource::Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return m_aGameConfig.Load(pRoot, pGameConfig, vecMessages);
}

void EntityManager::Provider::GameDataStorage::GameResource::Reset()
{
	m_nDestroyResourceManifestOffset = -1;
	m_nPrecacheEntitiesAndConfirmResourcesAreLoadedOffset = -1;
	m_nAllocGameResourceManifestOffset = -1;
	m_nAppendToAndCreateGameResourceManifestOffset = -1;
	m_nEntitySystemOffset = -1;
	m_nEntityManifestOffset = -1;
	m_nEntityManifestVFTableOffset = -1;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::GameResource::GetDestroyResourceManifestOffset() const
{
	return m_nDestroyResourceManifestOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::GameResource::GetPrecacheEntitiesAndConfirmResourcesAreLoadedOffset() const
{
	return m_nPrecacheEntitiesAndConfirmResourcesAreLoadedOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::GameResource::GetAllocGameResourceManifestOffset() const
{
	return m_nAllocGameResourceManifestOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::GameResource::GetAppendToAndCreateGameResourceManifestOffset() const
{
	return m_nAppendToAndCreateGameResourceManifestOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::GameResource::GetEntitySystemOffset() const
{
	return m_nEntitySystemOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::GameResource::GetEntityManifestOffset() const
{
	return m_nEntityManifestOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::GameResource::GetEntityManifestVFTableOffset() const
{
	return m_nEntityManifestVFTableOffset;
}
