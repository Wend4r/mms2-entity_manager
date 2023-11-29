#include "spawngroup.h"

#include <iserver.h>
#include <mathlib/mathlib.h>

extern INetworkServerService *g_pNetworkServerService;

EntityManager::SpawnGroup::~SpawnGroup()
{
	SpawnGroupHandle_t hSpawnGroup = this->m_hSpawnGroup;

	if(hSpawnGroup != INVALID_SPAWN_GROUP)
	{
		// engine->UnloadSpawnGroup(hSpawnGroup, kSGUO_None);
		INetworkGameServer *pNewServer = g_pNetworkServerService->GetIGameServer();

		if(pNewServer)
		{
			pNewServer->AsyncUnloadSpawnGroup(hSpawnGroup, kSGUO_None);
		}
	}
}

/*
EntityManager::SpawnGroup::Status EntityManager::SpawnGroup::GetStatus()
{
	SpawnGroupHandle_t hSpawnGroup = this->m_hSpawnGroup;

	if(hSpawnGroup != -1)
	{
		if(engine->IsSpawnGroupLoading(hSpawnGroup))
		{
			return kEMSGS_InProgress;
		}
		else if(engine->IsSpawnGroupLoaded(hSpawnGroup))
		{
			return kEMSGS_IsLoaded;
		}
		else
		{
			return kEMSGS_Broken;
		}
	}

	return kEMSGS_None;
}
*/

int EntityManager::SpawnGroup::GetStatus()
{
	SpawnGroupHandle_t hSpawnGroup = this->m_hSpawnGroup;

	if(hSpawnGroup != INVALID_SPAWN_GROUP)
	{
		INetworkGameServer *pNewServer = g_pNetworkServerService->GetIGameServer();

		if(pNewServer)
		{
			return pNewServer->GetSpawnGroupLoadingStatus(hSpawnGroup);
		}
	}

	return 0;
}

/*
bool EntityManager::SpawnGroup::IsResidentOrStreaming(SpawnGroupHandle_t hSpawnGroup)
{
	return engine->IsSpawnGroupLoading(hSpawnGroup) || engine->IsSpawnGroupLoaded(hSpawnGroup);
}
*/

bool EntityManager::SpawnGroup::IsResidentOrStreaming(SpawnGroupHandle_t hSpawnGroup)
{
	INetworkGameServer *pNewServer = g_pNetworkServerService->GetIGameServer();

	if(pNewServer)
	{
		return pNewServer->GetSpawnGroupLoadingStatus(hSpawnGroup) != 2;
	}

	return false;
}

const Vector &EntityManager::SpawnGroup::GetLandmarkOffset()
{
	return this->m_vecLandmarkOffset;
}

bool EntityManager::SpawnGroup::Start(const char *pLevelName, const char *pEntityLumpName, const char *pSpawnGroupFilterName, const char *pWorldGroupname, float flTimeoutInterval, const char *pLandmarkName, SpawnGroupHandle_t hOwner)
{
	INetworkGameServer *pNewServer = g_pNetworkServerService->GetIGameServer();

	if(!pNewServer)
	{
		return false;
	}

	this->m_sLevelName = pLevelName;
	this->m_sLandmarkName = pLandmarkName;

	if(pLevelName && pLevelName[0])
	{
		SpawnGroupHandle_t hSpawnGroup = pNewServer->FindSpawnGroupByName(pLevelName);

		if(IsResidentOrStreaming(hSpawnGroup))
		{
			return false;
		}
	}

	SpawnGroupDesc_t desc;

	desc.m_hOwner = hOwner;
	desc.m_pWorldOffsetCallback = this;
	desc.m_sWorldName = this->m_sLevelName;
	desc.m_sWorldMountName = this->m_sLevelName;
	desc.m_sEntityLumpName = pEntityLumpName;
	desc.m_sEntityFilterName = pSpawnGroupFilterName;
	desc.m_sDescriptiveName = pLandmarkName;
	desc.m_sWorldGroupname = pWorldGroupname;
	desc.m_flTimeoutInterval = flTimeoutInterval;
	desc.m_bCreateClientEntitiesOnLaterConnectingClients = true;

	pNewServer->LoadSpawnGroup(desc);

	return true;
}

matrix3x4_t EntityManager::SpawnGroup::ComputeWorldOrigin(const char *pWorldName, SpawnGroupHandle_t hSpawnGroup, IWorld *pWorld)
{
	matrix3x4_t res;

	res.SetToIdentity();
	res.SetOrigin(this->GetLandmarkOffset());

	return res;
}
