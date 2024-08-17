#include <entity_manager/provider_agent/resourcemanifest.hpp>
#include <entity_manager/provider/gameresource.hpp>

EntityManager::ResourceManifest::~ResourceManifest()
{
	this->Destroy();
}

bool EntityManager::ResourceManifest::Init(ResourceManifestLoadBehavior_t eBehavior, const char *pszAllocatorName, ResourceManifestLoadPriority_t ePriority)
{
	CGameResourceManifest *pNewManifest = ((EntityManager::CGameResourceServiceProvider *)g_pGameResourceServiceServer)->AllocGameResourceManifest(eBehavior, pszAllocatorName, ePriority);

	this->m_pStorage = pNewManifest;

	return pNewManifest != NULL;
}

bool EntityManager::ResourceManifest::Reinit(ResourceManifestLoadBehavior_t eBehavior, const char *pszAllocatorName, ResourceManifestLoadPriority_t ePriority)
{
	if(this->m_pStorage)
	{
		this->Destroy();
	}

	return this->Init();
}

bool EntityManager::ResourceManifest::Destroy()
{
	CGameResourceManifest *pDestroyManifest = this->m_pStorage;

	bool bResult = pDestroyManifest != NULL;

	if(bResult)
	{
		((EntityManager::CGameResourceServiceProvider *)g_pGameResourceServiceServer)->DestroyResourceManifest(pDestroyManifest);
	}

	return bResult;
}

bool EntityManager::ResourceManifest::Erect(ISpawnGroup *pSpawnGroup, int nCount, const EntitySpawnInfo_t *pEntities, const matrix3x4a_t *const vWorldOffset)
{
	// bool bResult = true;
	bool bResult = ((EntityManager::CGameResourceServiceProvider *)g_pGameResourceServiceServer)->AppendToAndCreateGameResourceManifest(this->m_pStorage, pSpawnGroup->GetHandle(), nCount, pEntities, vWorldOffset);

	if(bResult)
	{
		// pSpawnGroup->OnGameResourceManifestLoaded(NULL);
	}

	return bResult;
}

IEntityResourceManifest *EntityManager::ResourceManifest::GetEntityPart()
{
	IEntityResourceManifest *pEntityManifestVFT = nullptr;

	CEntityResourceManifest *pEntityManifest = ((CGameResourceManifestProvider *)this->m_pStorage)->GetEntityPart();

	if(pEntityManifest)
	{
		pEntityManifestVFT = ((CEntityResourceManifestProvider *)(pEntityManifest))->GetInterface();
	}

	return pEntityManifestVFT;
}
