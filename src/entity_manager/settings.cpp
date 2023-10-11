#include <stdio.h>

#include <tier0/dbg.h>
#include <tier0/platform.h>
#include <tier1/KeyValues.h>
#include <tier1/utlstring.h>
#include <tier1/utlvector.h>
#include <filesystem.h>

#include "placement/entitykeyvalues_provider.h"
#include "placement/entitysystem_provider.h"
#include "settings.h"

#define ENTITY_MANAGER_MAP_CONFIG_DIR "configs/maps"
#define ENTITY_MANAGER_MAP_CONFIG_WORLD_FILE "world.vdf"

extern IFileSystem *filesystem;

extern CGameEntitySystem *g_pEntitySystem;

bool EntityManagerSpace::Settings::Init(char *psError, size_t nMaxLength)
{
	this->m_pWorld = new KeyValues("World");

	return true;
}

bool EntityManagerSpace::Settings::Load(const char *pszBasePath, const char *pszMapName, char *psError, size_t nMaxLength)
{
	char sBaseConfigsDir[MAX_PATH];

	snprintf((char *)sBaseConfigsDir, sizeof(sBaseConfigsDir), 
#ifdef PLATFORM_WINDOWS
		"%s\\%s\\%s",
#else
		"%s/%s/%s",
#endif
		pszBasePath, ENTITY_MANAGER_MAP_CONFIG_DIR, pszMapName);

	bool bResult = this->LoadWorld((const char *)sBaseConfigsDir, psError, nMaxLength);

	if(bResult)
	{
		// ...
	}

	return bResult;
}

void EntityManagerSpace::Settings::Clear()
{
	this->m_pWorld->Clear();
}

void EntityManagerSpace::Settings::Destroy()
{
	delete this->m_pWorld;
	this->m_pWorld = nullptr;
}

bool EntityManagerSpace::Settings::LoadWorld(const char *pszBaseConfigsDir, char *psError, size_t nMaxLength)
{
	char sConfigFile[MAX_PATH];

	snprintf((char *)sConfigFile, sizeof(sConfigFile), 
#ifdef PLATFORM_WINDOWS
		"%s\\%s",
#else
		"%s/%s",
#endif
		pszBaseConfigsDir, ENTITY_MANAGER_MAP_CONFIG_WORLD_FILE);

	KeyValues *pWorldValues = this->m_pWorld;

	bool bResult = pWorldValues->LoadFromFile(filesystem, (const char *)sConfigFile);

	if(bResult)
	{
		CUtlVector<CUtlString> vecErrors;

		CEntitySystemProvider *pEntitySystem = (CEntitySystemProvider *)g_pEntitySystem;

		{
			char sEntityError[256];

			CBaseEntity *pEntity;

			CEntityKeyValues *pEntityKeyValues;

			FOR_EACH_SUBKEY(pWorldValues, pEntityValues)
			{
				if(this->LoadWorldEntity(pEntityValues, pEntity, pEntityKeyValues, (char *)sEntityError, sizeof(sEntityError)))
				{
					pEntitySystem->QueueSpawnEntity(pEntity->m_pEntity, pEntityKeyValues);
				}
				else
				{
					vecErrors.AddToTail(CUtlString((const char *)sEntityError));
				}
			}

			pEntitySystem->ExecuteQueuedCreation();
		}

		// Print errors.
		FOR_EACH_VEC(vecErrors, i)
		{
			Warning("Failed to create entity: %s", vecErrors[i].Get());
		}
	}
	else if(psError)
	{
		snprintf(psError, nMaxLength, "Can't to load KeyValue from \"%s\" file", sConfigFile);
	}

	return bResult;
}

bool EntityManagerSpace::Settings::LoadWorldEntity(KeyValues *pEntityValues, CBaseEntity *&pResultEntity, CEntityKeyValues *&pResultKeyValues, char *psError, size_t nMaxLength)
{
	const char *pszClassname = pEntityValues->GetString("classname");

	CEntityIndex iForceEdictIndex = CEntityIndex(-1);

	pResultEntity = (CBaseEntity *)((CEntitySystemProvider *)g_pEntitySystem)->CreateEntity(iForceEdictIndex, pszClassname, ENTITY_NETWORKING_MODE_DEFAULT, (SpawnGroupHandle_t)-1, -1, false);

	bool bResult = pResultEntity != nullptr;

	if(bResult)
	{
		int iIndex = pResultEntity->m_pEntity->m_EHandle.GetEntryIndex();

		DebugMsg("Created \"%s\" (requested edict index is %d, result index is %d) entity\n", pszClassname, iForceEdictIndex.Get(), iIndex);

		CEntityKeyValuesProvider *pNewKeyValues = (CEntityKeyValuesProvider *)CEntityKeyValuesProvider::Create();

		bResult = pNewKeyValues != nullptr;

		if(bResult)
		{
			FOR_EACH_VALUE(pEntityValues, pKeyValue)
			{
				const char *pszKey = pKeyValue->GetName();

				void *pAttr = pNewKeyValues->GetAttribute({ /* Meow */ MurmurHash2LowerCase(pszKey, 0x31415926u), pszKey});

				if(pAttr)
				{
					const char *pszValue = pKeyValue->GetString(NULL);

					DebugMsg("%d: \"%s\" has \"%s\" value\n", iIndex, pszKey, pszValue);

					pNewKeyValues->SetAttributeValue(pAttr, pszValue);
				}
				else
				{
					Warning("Failed to get \"%s\" attribute ", pszKey);
				}
			}

			pResultKeyValues = pNewKeyValues;
		}
		else if(psError)
		{
			snprintf(psError, nMaxLength, "Can't create key values of \"%s\"", pszClassname);
		}
	}
	else if(psError)
	{
		snprintf(psError, nMaxLength, "Can't create \"%s\"", pszClassname);
	}

	return bResult;
}
