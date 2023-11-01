#include "provider.h"
#include "provider_agent.h"

extern EntityManager::ProviderAgent *g_pEntityManagerProviderAgent;

EntityManager::Provider::GameDataStorage::GameResource::GameResource()
{
	{
		auto &aCallbacks = this->m_aOffsetCallbacks;

		aCallbacks.Insert("CGameResourceService::DestroyResourceManifest", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nDestroyResourceManifestOffset = nOffset;
		});
		aCallbacks.Insert("CGameResourceService::PrecacheEntitiesAndConfirmResourcesAreLoaded", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nPrecacheEntitiesAndConfirmResourcesAreLoadedOffset = nOffset;
		});
		aCallbacks.Insert("CGameResourceService::m_pEntitySystem", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nEntitySystemOffset = nOffset;
			g_pEntityManagerProviderAgent->NotifyEntitySystemUpdated();
		});
		aCallbacks.Insert("CGameResourceService::AllocGameResourceManifest", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nAllocGameResourceManifestOffset = nOffset;
		});
		aCallbacks.Insert("CGameResourceService::AppendToAndCreateGameResourceManifest", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nAppendToAndCreateGameResourceManifestOffset = nOffset;
		});

		this->m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::GameResource::Load(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aGameConfig.Load(pRoot, pGameConfig, psError, nMaxLength);
}

void EntityManager::Provider::GameDataStorage::GameResource::Reset()
{
	this->m_nDestroyResourceManifestOffset = -1;
	this->m_nPrecacheEntitiesAndConfirmResourcesAreLoadedOffset = -1;
	this->m_nAllocGameResourceManifestOffset = -1;
	this->m_nAppendToAndCreateGameResourceManifestOffset = -1;
	this->m_nEntitySystemOffset = -1;
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