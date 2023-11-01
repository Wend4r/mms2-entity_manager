#include "resourcemanifest.h"

#include <entity_manager/provider/gameresource.h>

extern IGameResourceServiceServer *g_pGameResourceServiceServer;

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

bool EntityManager::ResourceManifest::Erect(SpawnGroupHandle_t hSpawnGroup, int nCount, const EntitySpawnInfo_t *pEntities, const matrix3x4a_t *const vWorldOffset)
{
	bool bResult = ((EntityManager::CGameResourceServiceProvider *)g_pGameResourceServiceServer)->AppendToAndCreateGameResourceManifest(this->m_pStorage, hSpawnGroup, nCount, pEntities, vWorldOffset);

	if(bResult)
	{
		// pSpawnGroup->OnGameResourceManifestLoaded(this->m_pStorage, 0, NULL);
	}

	return bResult;
}


