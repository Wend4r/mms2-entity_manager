#include "provider.h"
#include "gamedata.h"

#include <entity2/entitysystem.h>

extern EntityManager::GameData *g_pEntityManagerGameData;

bool EntityManager::Provider::Init(char *psError, size_t nMaxLength)
{
	return true;
}

bool EntityManager::Provider::Load(char *psError, size_t nMaxLength)
{
	bool bResult = this->LoadGameData(psError, nMaxLength);

	if(bResult)
	{
		// Next load ...
	}

	return bResult;
}

void EntityManager::Provider::Destroy()
{
	// ...
}

bool EntityManager::Provider::LoadGameData(char *psError, size_t nMaxLength)
{
	bool bResult = this->LoadEntitySystemGameData(psError, nMaxLength);

	if(bResult)
	{
		bResult = this->LoadEntityKeyValuesGameData(psError, nMaxLength);

		if(bResult)
		{
			bResult = this->LoadSpawnGroupGameData(psError, nMaxLength);
		}
	}

	return bResult;
}

bool EntityManager::Provider::LoadEntityKeyValuesGameData(char *psError, size_t nMaxLength)
{
	bool bResult = true;
	
	{
		const char *pszSignatureName = "CEntityKeyValues::CEntityKeyValues";

		CMemory pResult = g_pEntityManagerGameData->GetEntityKeyValuesAddress(pszSignatureName);

		bResult = pResult;

		if(bResult)
		{
			this->m_aData.m_aEntityKeyValues.m_pfnEntityKeyValues = pResult.RCast<GameData::EntityKeyValues::EntityKeyValuesFuncType>();

			pszSignatureName = "CEntityKeyValues::GetAttribute";
			pResult = g_pEntityManagerGameData->GetEntityKeyValuesAddress(pszSignatureName);
			bResult = pResult;

			if(bResult)
			{
				this->m_aData.m_aEntityKeyValues.m_pfnGetAttribute = pResult.RCast<GameData::EntityKeyValues::GetAttributeFuncType>();

				pszSignatureName = "CEntityKeyValuesAttribute::GetValueString";
				pResult = g_pEntityManagerGameData->GetEntityKeyValuesAddress(pszSignatureName);
				bResult = pResult;

				if(bResult)
				{
					this->m_aData.m_aEntityKeyValues.m_pfnGetValueString = pResult.RCast<GameData::EntityKeyValues::GetValueStringFuncType>();

					pszSignatureName = "CEntityKeyValues::SetAttributeValue";
					pResult = g_pEntityManagerGameData->GetEntityKeyValuesAddress(pszSignatureName);
					bResult = pResult;

					if(bResult)
					{
						this->m_aData.m_aEntityKeyValues.m_pfnSetAttributeValue = pResult.RCast<GameData::EntityKeyValues::SetAttributeValueFuncType>();
					}
				}
			}
		}

		if(!bResult && psError)
		{
			snprintf(psError, nMaxLength, "Failed to get \"%s\" signature", pszSignatureName);
		}
	}

	if(bResult)
	{
		const char *pszOffsetName = "sizeof(CEntityKeyValues)";

		ptrdiff_t nResult = g_pEntityManagerGameData->GetEntityKeyValuesOffset(pszOffsetName);

		bResult = nResult != -1;

		if(bResult)
		{
			this->m_aData.m_aEntityKeyValues.m_nSizeof = nResult;

			pszOffsetName = "CEntityKeyValues::m_nRefCount";
			nResult = g_pEntityManagerGameData->GetEntityKeyValuesOffset(pszOffsetName);
			bResult = nResult != -1;

			if(bResult)
			{
				this->m_aData.m_aEntityKeyValues.m_nRefCountOffset = nResult;
			}
		}

		if(!bResult && psError)
		{
			snprintf(psError, nMaxLength, "Failed to get \"%s\" offset", pszOffsetName);
		}
	}

	return bResult;
}


bool EntityManager::Provider::LoadEntitySystemGameData(char *psError, size_t nMaxLength)
{
	bool bResult = true;

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
	}

	if(bResult)
	{
		const char *pszOffsetName = "CEntitySystem::m_pCurrentManifest";

		ptrdiff_t nResult = g_pEntityManagerGameData->GetEntitySystemOffset(pszOffsetName);

		bResult = nResult != -1;

		if(bResult)
		{
			this->m_aData.m_aEntitySystem.m_nCurrentManifestOffset = nResult;

			pszOffsetName = "CEntitySystem::m_aKeyValuesMemoryPool";
			nResult = g_pEntityManagerGameData->GetEntitySystemOffset(pszOffsetName);
			bResult = nResult != -1;

			if(bResult)
			{
				this->m_aData.m_aEntitySystem.m_nKeyValuesMemoryPoolOffset = nResult;
			}
		}

		if(!bResult && psError)
		{
			snprintf(psError, nMaxLength, "Failed to get \"%s\" offset", pszOffsetName);
		}
	}

	return bResult;
}

bool EntityManager::Provider::LoadSpawnGroupGameData(char *psError, size_t nMaxLength)
{
	const char *pszOffsetName = "CSpawnGroupMgrGameSystem::m_SpawnGroups";

	ptrdiff_t nResult = g_pEntityManagerGameData->GetSpawnGroupOffset(pszOffsetName);

	bool bResult = nResult != -1;

	if(bResult)
	{
		this->m_aData.m_aSpawnGroup.m_nMgrGameSystemSpawnGroupsOffset = nResult;
	}

	if(!bResult && psError)
	{
		snprintf(psError, nMaxLength, "Failed to get \"%s\" offset", pszOffsetName);
	}

	return bResult;
}
