#include <provider.hpp>
#include "provider_agent.hpp"

DLL_IMPORT EntityManager::ProviderAgent *g_pEntityManagerProviderAgent;

EntityManager::Provider::GameDataStorage::GameResource::GameResource()
{
	{
		auto &aCallbacks = this->m_aOffsetCallbacks;

		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CGameResourceService::DestroyResourceManifest"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			this->m_nDestroyResourceManifestOffset = nOffset;
		});
		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CGameResourceService::PrecacheEntitiesAndConfirmResourcesAreLoaded"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			this->m_nPrecacheEntitiesAndConfirmResourcesAreLoadedOffset = nOffset;
		});
		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CGameResourceService::AllocGameResourceManifest"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			this->m_nAllocGameResourceManifestOffset = nOffset;
		});
		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CGameResourceService::AppendToAndCreateGameResourceManifest"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			this->m_nAppendToAndCreateGameResourceManifestOffset = nOffset;
		});
		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CGameResourceService::m_pEntitySystem"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			this->m_nEntitySystemOffset = nOffset;
			g_pEntityManagerProviderAgent->NotifyEntitySystemUpdated();
		});
		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CGameResourceService::m_pEntityManifest"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			this->m_nEntityManifestOffset = nOffset;
		});
		aCallbacks.Insert(this->m_aGameConfig.GetSymbol("CEntityResourceManifest::`vftable'"), [this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			this->m_nEntityManifestVFTableOffset = nOffset;
		});

		this->m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::GameResource::Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return this->m_aGameConfig.Load(pRoot, pGameConfig, vecMessages);
}

void EntityManager::Provider::GameDataStorage::GameResource::Reset()
{
	this->m_nDestroyResourceManifestOffset = -1;
	this->m_nPrecacheEntitiesAndConfirmResourcesAreLoadedOffset = -1;
	this->m_nAllocGameResourceManifestOffset = -1;
	this->m_nAppendToAndCreateGameResourceManifestOffset = -1;
	this->m_nEntitySystemOffset = -1;
	this->m_nEntityManifestOffset = -1;
	this->m_nEntityManifestVFTableOffset = -1;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::GameResource::GetDestroyResourceManifestOffset() const
{
	return this->m_nDestroyResourceManifestOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::GameResource::GetPrecacheEntitiesAndConfirmResourcesAreLoadedOffset() const
{
	return this->m_nPrecacheEntitiesAndConfirmResourcesAreLoadedOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::GameResource::GetAllocGameResourceManifestOffset() const
{
	return this->m_nAllocGameResourceManifestOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::GameResource::GetAppendToAndCreateGameResourceManifestOffset() const
{
	return this->m_nAppendToAndCreateGameResourceManifestOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::GameResource::GetEntitySystemOffset() const
{
	return this->m_nEntitySystemOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::GameResource::GetEntityManifestOffset() const
{
	return this->m_nEntityManifestOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::GameResource::GetEntityManifestVFTableOffset() const
{
	return this->m_nEntityManifestVFTableOffset;
}
