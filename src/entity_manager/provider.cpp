#include "provider.h"
#include "gamedata.h"

#include <entity2/entitysystem.h>

extern CGameEntitySystem *g_pEntitySystem;

extern EntityManagerSpace::GameData *g_pEntityManagerGameData;

bool EntityManagerSpace::Provider::Init(char *psError, size_t nMaxLength)
{
	return true;
}

bool EntityManagerSpace::Provider::Load(char *psError, size_t nMaxLength)
{
	bool bResult = this->LoadGameData(psError, nMaxLength);

	if(bResult)
	{
		// Next load ...
	}

	return bResult;
}

void EntityManagerSpace::Provider::Destroy()
{
	// ...
}

bool EntityManagerSpace::Provider::LoadGameData(char *psError, size_t nMaxLength)
{
	bool bResult = this->LoadEntitySystemGameData(psError, nMaxLength);

	if(bResult)
	{
		bResult = this->LoadEntityKeyValuesGameData(psError, nMaxLength);
	}

	return bResult;
}

bool EntityManagerSpace::Provider::LoadEntityKeyValuesGameData(char *psError, size_t nMaxLength)
{
	const char *pszSignatureName = "CEntityKeyValues::CEntityKeyValues";

	CMemory pResult = g_pEntityManagerGameData->GetEntityKeyValuesAddress(pszSignatureName);

	bool bResult = pResult;

	if(bResult)
	{
		this->m_aData.m_aEntityKeyValues.m_pfnEntityKeyValues = pResult.RCast<GameData::EntityKeyValues::EntityKeyValuesFuncType>();

		pszSignatureName = "CEntityKeyValues::GetAttribute";
		pResult = g_pEntityManagerGameData->GetEntityKeyValuesAddress(pszSignatureName);
		bResult = pResult;

		if(bResult)
		{
			this->m_aData.m_aEntityKeyValues.m_pfnGetAttribute = pResult.RCast<GameData::EntityKeyValues::GetAttributeFuncType>();

			pszSignatureName = "CEntityKeyValues::SetAttributeValue";
			pResult = g_pEntityManagerGameData->GetEntityKeyValuesAddress(pszSignatureName);
			bResult = pResult;

			if(bResult)
			{
				this->m_aData.m_aEntityKeyValues.m_pfnSetAttributeValue = pResult.RCast<GameData::EntityKeyValues::SetAttributeValueFuncType>();
			}
		}
	}

	if(!bResult && psError)
	{
		snprintf(psError, nMaxLength, "Failed to get \"%s\" signature", pszSignatureName);
	}

	return bResult;
}


bool EntityManagerSpace::Provider::LoadEntitySystemGameData(char *psError, size_t nMaxLength)
{
	const char *pszSignatureName = "CEntitySystem::CreateEntity";

	CMemory pResult = g_pEntityManagerGameData->GetEntitySystemAddress(pszSignatureName);

	bool bResult = pResult;

	if(bResult)
	{
		this->m_aData.m_aEntitySystem.m_pfnCreateEntity = pResult.RCast<GameData::EntitySystem::CreateEntityFuncType>();

		pszSignatureName = "CEntitySystem::QueueSpawnEntity";
		pResult = g_pEntityManagerGameData->GetEntitySystemAddress(pszSignatureName);
		bResult = pResult;

		if(bResult)
		{
			this->m_aData.m_aEntitySystem.m_pfnQueueSpawnEntity = pResult.RCast<GameData::EntitySystem::QueueSpawnEntityFuncType>();

			pszSignatureName = "CEntitySystem::ExecuteQueuedCreation";
			pResult = g_pEntityManagerGameData->GetEntitySystemAddress(pszSignatureName);
			bResult = pResult;

			if(bResult)
			{
				this->m_aData.m_aEntitySystem.m_pfnExecuteQueuedCreation = pResult.RCast<GameData::EntitySystem::ExecuteQueuedCreationFuncType>();
			}
		}
	}

	if(!bResult && psError)
	{
		snprintf(psError, nMaxLength, "Failed to get \"%s\" signature", pszSignatureName);
	}

	return bResult;
}
