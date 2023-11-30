#include "spawngroup.h"

#include <iserver.h>
#include <mathlib/mathlib.h>

extern INetworkServerService *g_pNetworkServerService;

EntityManager::SpawnGroup::~SpawnGroup()
{
	this->Unload();
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

	return 2;
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
		return pNewServer->GetSpawnGroupLoadingStatus(hSpawnGroup) < 2;
	}

	return false;
}

const Vector &EntityManager::SpawnGroup::GetLandmarkOffset()
{
	return this->m_vecLandmarkOffset;
}

bool EntityManager::SpawnGroup::Start(SpawnGroupDesc_t &aDesc, const Vector &vecLandmarkOffset)
{
	if(this->GetStatus() < 2)
	{
		AssertMsg4(0, "Accept to start an already existing spawn group: %s (%s) vs %s (%s)", this->m_sLevelName.Get(), this->m_sLandmarkName.Get(), aDesc.m_sWorldName.Get(), aDesc.m_sDescriptiveName.Get());

		return false;
	}

	INetworkGameServer *pNewServer = g_pNetworkServerService->GetIGameServer();

	if(!pNewServer)
	{
		return false;
	}

	{
		const char *pLevelName = aDesc.m_sWorldName.Get();

		if(pLevelName && pLevelName[0])
		{
			SpawnGroupHandle_t hSpawnGroup = pNewServer->FindSpawnGroupByName(pLevelName);

			if(IsResidentOrStreaming(hSpawnGroup))
			{
				return false;
			}
		}
	}

	this->m_sLevelName = aDesc.m_sWorldName;
	this->m_sLandmarkName = aDesc.m_sDescriptiveName;
	this->m_vecLandmarkOffset = vecLandmarkOffset;

	aDesc.m_pWorldOffsetCallback = this;

	pNewServer->LoadSpawnGroup(aDesc);

	return true;
}

bool EntityManager::SpawnGroup::Unload()
{
	SpawnGroupHandle_t hSpawnGroup = this->m_hSpawnGroup;

	bool bResult = hSpawnGroup != INVALID_SPAWN_GROUP;

	if(bResult)
	{
		// engine->UnloadSpawnGroup(hSpawnGroup, kSGUO_None);
		INetworkGameServer *pNewServer = g_pNetworkServerService->GetIGameServer();

		bResult = pNewServer != NULL;

		if(bResult)
		{
			pNewServer->AsyncUnloadSpawnGroup(hSpawnGroup, kSGUO_None);
		}
	}

	return bResult;
}

matrix3x4_t EntityManager::SpawnGroup::ComputeWorldOrigin(const char *pWorldName, SpawnGroupHandle_t hSpawnGroup, IWorld *pWorld)
{
	this->m_hSpawnGroup = hSpawnGroup;

	matrix3x4_t res;

	res.SetToIdentity();
	res.SetOrigin(this->GetLandmarkOffset());

	return res;
}
