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
	return (CBaseEntity *)(g_pEntitySystem->*(this->m_pfnEntitySystemCreateEntity))((SpawnGroupHandle_t)-1, pszClassName, ENTITY_NETWORKING_MODE_DEFAULT, iForceEdictIndex, -1, nullptr, 0);
}

bool EntityManagerSpace::Placement::LoadGameData(char *psError, size_t nMaxLength)
{
	const char *pszSignatureName = "CEntitySystem::CreateEntity";

	CMemory pCreateEntity = g_pEntityManagerGameData->GetEntitySystemAddress(pszSignatureName);

	bool bResult = pCreateEntity;

	if(bResult)
	{
		this->m_pfnEntitySystemCreateEntity = pCreateEntity.FCast<CEntitySystemCaller::CreateEntity>();
	}
	else if(psError)
	{
		snprintf(psError, nMaxLength, "Failed to get \"%s\" signature", pszSignatureName);
	}

	return bResult;
}
