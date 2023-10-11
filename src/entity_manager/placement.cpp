#include "placement.h"
#include "gamedata.h"

#include <entity2/entitysystem.h>

extern CGameEntitySystem *g_pEntitySystem;

extern EntityManagerSpace::GameData *g_pEntityManagerGameData;

bool EntityManagerSpace::Placement::Init(char *psError, size_t nMaxLength)
{
	return true;
}

bool EntityManagerSpace::Placement::Load(char *psError, size_t nMaxLength)
{
	bool bResult = this->LoadGameData(psError, nMaxLength);

	if(bResult)
	{
		// Next load ...
	}

	return bResult;
}

void EntityManagerSpace::Placement::Destroy()
{
	// ...
}

CBaseEntity *EntityManagerSpace::Placement::CreateEntity(const char *pszClassName, CEntityIndex iForceEdictIndex)
{
	CEntityInstance *pEntity = ((CEntitySystemProvider *)g_pEntitySystem)->CreateEntity(iForceEdictIndex, pszClassName, ENTITY_NETWORKING_MODE_DEFAULT, (SpawnGroupHandle_t)-1, -1, false);

	if(pEntity)
	{
		DebugMsg("Created \"%s\" (requested edict index is %d, result index is %d) entity\n", pszClassName, iForceEdictIndex.Get(), pEntity->m_pEntity->m_EHandle.GetEntryIndex());
	}
	else
	{
		DevWarning("Failed to create \"%s\" (requested edict index is %d) entity\n", pszClassName, iForceEdictIndex.Get());
	}

	return (CBaseEntity *)pEntity;
}

bool EntityManagerSpace::Placement::LoadGameData(char *psError, size_t nMaxLength)
{
	const char *pszSignatureName = "CEntitySystem::CreateEntity";

	CMemory pCreateEntity = g_pEntityManagerGameData->GetEntitySystemAddress(pszSignatureName);

	bool bResult = pCreateEntity;

	if(bResult)
	{
		this->m_aData.m_aEntitySystem.m_pfnCreateEntity = pCreateEntity.RCast<GameData::EntitySystem::CreateEntityFuncType>();
	}
	else if(psError)
	{
		snprintf(psError, nMaxLength, "Failed to get \"%s\" signature", pszSignatureName);
	}

	return bResult;
}
