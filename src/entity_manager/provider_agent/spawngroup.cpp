#include <entity_manager/provider_agent/spawngroup.hpp>

#include <iserver.h>
#include <mathlib/mathlib.h>

EntityManager::CSpawnGroupInstance::~CSpawnGroupInstance()
{
	if(m_hSpawnGroup != INVALID_SPAWN_GROUP)
	{
		Unload();
	}
}

/*
EntityManager::CSpawnGroupInstance::Status EntityManager::CSpawnGroupInstance::GetStatus()
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

/*
bool EntityManager::CSpawnGroupInstance::IsResidentOrStreaming(SpawnGroupHandle_t hSpawnGroup)
{
	return engine->IsSpawnGroupLoading(hSpawnGroup) || engine->IsSpawnGroupLoaded(hSpawnGroup);
}
*/

bool EntityManager::CSpawnGroupInstance::IsResidentOrStreaming(SpawnGroupHandle_t hSpawnGroup)
{
	INetworkGameServer *pNewServer = g_pNetworkServerService->GetIGameServer();

	if(pNewServer)
	{
		return pNewServer->GetSpawnGroupLoadingStatus(hSpawnGroup) < 2;
	}

	return false;
}

bool EntityManager::CSpawnGroupInstance::Load(const SpawnGroupDesc_t &aDesc, const Vector &vecLandmarkOffset)
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
	m_sLocalFixupName = aDesc.m_sLocalNameFixup;
	m_vecLandmarkOffset = vecLandmarkOffset;

	SpawnGroupDesc_t aOwnDesc = aDesc;

	aOwnDesc.m_pWorldOffsetCallback = this;

	pNewServer->LoadSpawnGroup(aOwnDesc);

	return true;
}

bool EntityManager::CSpawnGroupInstance::Unload()
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

void EntityManager::CSpawnGroupInstance::OnSpawnGroupAllocated(SpawnGroupHandle_t hSpawnGroup, ISpawnGroup *pSpawnGroup)
{
	m_pSpawnGroup = pSpawnGroup;
	m_hSpawnGroup = hSpawnGroup;

	for(auto *it : m_vecNotificationsCallbacks)
	{
		it->OnSpawnGroupAllocated(hSpawnGroup, pSpawnGroup);
	}
}


void EntityManager::CSpawnGroupInstance::OnSpawnGroupCreateLoading(SpawnGroupHandle_t hSpawnGroup, CMapSpawnGroup *pMapSpawnGroup, bool bSynchronouslySpawnEntities, bool bConfirmResourcesLoaded, CUtlVector<const CEntityKeyValues *> &vecKeyValues)
{
	m_pMapSpawnGroup = pMapSpawnGroup;

	for(auto *it : m_vecNotificationsCallbacks)
	{
		it->OnSpawnGroupCreateLoading(hSpawnGroup, pMapSpawnGroup, bSynchronouslySpawnEntities, bConfirmResourcesLoaded, vecKeyValues);
	}
}

void EntityManager::CSpawnGroupInstance::OnSpawnGroupDestroyed(SpawnGroupHandle_t hSpawnGroup)
{
	for(auto *it : m_vecNotificationsCallbacks)
	{
		it->OnSpawnGroupDestroyed(hSpawnGroup);
	}
}

void EntityManager::CSpawnGroupInstance::AddNotificationsListener(ISpawnGroupNotifications *pNotifications)
{
	m_vecNotificationsCallbacks.AddToTail(pNotifications);
}

bool EntityManager::CSpawnGroupInstance::RemoveNotificationsListener(ISpawnGroupNotifications *pNotifications)
{
	return m_vecNotificationsCallbacks.FindAndFastRemove(pNotifications);
}

int EntityManager::CSpawnGroupInstance::GetStatus() const
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

ISpawnGroup *EntityManager::CSpawnGroupInstance::GetSpawnGroup() const
{
	return m_pSpawnGroup;
}

CMapSpawnGroup *EntityManager::CSpawnGroupInstance::GetMapSpawnGroup() const
{
	return m_pMapSpawnGroup;
}

SpawnGroupHandle_t EntityManager::CSpawnGroupInstance::GetSpawnGroupHandle() const
{
	return m_hSpawnGroup;
}

const char *EntityManager::CSpawnGroupInstance::GetLevelName() const
{
	return m_sLevelName.Get();
}

const char *EntityManager::CSpawnGroupInstance::GetLandmarkName() const
{
	return m_sLandmarkName.Get();
}

const char *EntityManager::CSpawnGroupInstance::GetLocalFixupName() const
{
	return m_sLocalFixupName.Get();
}

const Vector &EntityManager::CSpawnGroupInstance::GetLandmarkOffset() const
{
	return m_vecLandmarkOffset;
}


matrix3x4_t EntityManager::CSpawnGroupInstance::ComputeWorldOrigin(const char *pWorldName, SpawnGroupHandle_t hSpawnGroup, IWorld *pWorld)
{
	matrix3x4_t res;

	res.SetToIdentity();
	res.SetOrigin(GetLandmarkOffset());

	return res;
}
