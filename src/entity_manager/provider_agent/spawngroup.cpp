#include <entity_manager/provider_agent/spawngroup.hpp>

#include <iserver.h>
#include <mathlib/mathlib.h>

EntityManager::SpawnGroup::~SpawnGroup()
{
	Unload();
}

/*
EntityManager::SpawnGroup::Status EntityManager::SpawnGroup::GetStatus()
{
	SpawnGroupHandle_t hSpawnGroup = m_hSpawnGroup;

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

int EntityManager::SpawnGroup::GetStatus() const
{
	SpawnGroupHandle_t hSpawnGroup = m_hSpawnGroup;

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

SpawnGroupHandle_t EntityManager::SpawnGroup::GetAllocatedSpawnGroup() const
{
	return m_hSpawnGroup;
}

const char *EntityManager::SpawnGroup::GetLevelName() const
{
	return m_sLevelName.Get();
}

const char *EntityManager::SpawnGroup::GetLandmarkName() const
{
	return m_sLandmarkName.Get();
}

const Vector &EntityManager::SpawnGroup::GetLandmarkOffset() const
{
	return m_vecLandmarkOffset;
}

bool EntityManager::SpawnGroup::Start(const SpawnGroupDesc_t &aDesc, const Vector &vecLandmarkOffset)
{
	if(GetStatus() < 2)
	{
		AssertMsg4(0, "Accept to start an already existing spawn group: %s (%s) vs %s (%s)", m_sLevelName.Get(), m_sLandmarkName.Get(), aDesc.m_sWorldName.Get(), aDesc.m_sDescriptiveName.Get());

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

	m_sLevelName = aDesc.m_sWorldName;
	m_sLandmarkName = aDesc.m_sDescriptiveName;
	m_vecLandmarkOffset = vecLandmarkOffset;

	SpawnGroupDesc_t aOwnDesc = aDesc;

	aOwnDesc.m_pWorldOffsetCallback = this;

	pNewServer->LoadSpawnGroup(aOwnDesc);

	return true;
}

bool EntityManager::SpawnGroup::Unload()
{
	SpawnGroupHandle_t hSpawnGroup = m_hSpawnGroup;

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

void EntityManager::SpawnGroup::NotifyAllocateSpawnGroup(SpawnGroupHandle_t handle, ISpawnGroup *pSpawnGroup)
{
	m_hSpawnGroup = handle;
}

void EntityManager::SpawnGroup::NotifyDestroySpawnGroup(SpawnGroupHandle_t handle)
{
	// ...
}

matrix3x4_t EntityManager::SpawnGroup::ComputeWorldOrigin(const char *pWorldName, SpawnGroupHandle_t hSpawnGroup, IWorld *pWorld)
{
	matrix3x4_t res;

	res.SetToIdentity();
	res.SetOrigin(GetLandmarkOffset());

	return res;
}
