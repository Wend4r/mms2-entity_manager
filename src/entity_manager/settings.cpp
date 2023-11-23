#include <stdio.h>

#include "settings.h"

#include <tier0/dbg.h>
#include <tier0/platform.h>
#include <tier0/keyvalues.h>
#include <tier0/strtools.h>
#include <tier0/utlstring.h>
#include <tier1/utlvector.h>
#include <filesystem.h>

#include "provider_agent.h"

#define ENTITY_MANAGER_MAP_CONFIG_DIR "configs"
#define ENTITY_MANAGER_MAP_CONFIG_SPAWNGROUPS_DIR "spawngroups"
#define ENTITY_MANAGER_MAP_CONFIG_WORLD_FILE "world.vdf"

extern IFileSystem *filesystem;

extern EntityManager::ProviderAgent *g_pEntityManagerProviderAgent;

bool EntityManager::Settings::Init(char *psError, size_t nMaxLength)
{
	this->m_pWorld = new KeyValues("World");

	return true;
}

bool EntityManager::Settings::Load(SpawnGroupHandle_t hSpawnGroup, const char *pszBasePath, const char *pszSpawnGroupName, char *psError, size_t nMaxLength, Logger::Scope *pDetails, Logger::Scope *pWarnings)
{
	char sBaseConfigsDir[MAX_PATH];

	snprintf((char *)sBaseConfigsDir, sizeof(sBaseConfigsDir), "%s" CORRECT_PATH_SEPARATOR_S "%s" CORRECT_PATH_SEPARATOR_S "%s" CORRECT_PATH_SEPARATOR_S "%s", pszBasePath, ENTITY_MANAGER_MAP_CONFIG_DIR, ENTITY_MANAGER_MAP_CONFIG_SPAWNGROUPS_DIR, pszSpawnGroupName);

	bool bResult = this->LoadWorld(hSpawnGroup, (const char *)sBaseConfigsDir, psError, nMaxLength, pDetails, pWarnings);

	if(bResult)
	{
		// ...
	}

	return bResult;
}

void EntityManager::Settings::Clear()
{
	// Todo.
}

void EntityManager::Settings::Destroy()
{
	delete this->m_pWorld;
	this->m_pWorld = nullptr;
}

bool EntityManager::Settings::LoadWorld(SpawnGroupHandle_t hSpawnGroup, const char *pszBaseConfigsDir, char *psError, size_t nMaxLength, Logger::Scope *pDetails, Logger::Scope *pWarnings)
{
	char sConfigFile[MAX_PATH];

	snprintf((char *)sConfigFile, sizeof(sConfigFile), "%s" CORRECT_PATH_SEPARATOR_S "%s", pszBaseConfigsDir, ENTITY_MANAGER_MAP_CONFIG_WORLD_FILE);

	KeyValues *pWorldValues = this->m_pWorld;

	bool bResult = pWorldValues->LoadFromFile(filesystem, (const char *)sConfigFile);

	if(bResult)
	{
		if(pDetails)
		{
			pDetails->PushFormat("- Loading \"%s\" config file (at #%d sg) -", sConfigFile, hSpawnGroup);
		}

		FOR_EACH_SUBKEY(pWorldValues, pSubValues)
		{
			const char *pszSection = pSubValues->GetName();

			if(!V_strcmp(pszSection, "entity"))
			{
				if(pDetails)
				{
					pDetails->Push("-- Queue entity --");

					auto aEntityDetails = Logger::Scope(LOGGER_COLOR_KEYVALUES, "\t");

					if(g_pEntityManagerProviderAgent->DumpOldKeyValues(pSubValues, aEntityDetails, pWarnings))
					{
						pDetails->PushFormat(LOGGER_COLOR_KEYVALUES, "\"%s\"", pszSection);
						pDetails->Push(LOGGER_COLOR_KEYVALUES, "{");
						*pDetails += aEntityDetails;
						pDetails->Push(LOGGER_COLOR_KEYVALUES, "}");
					}
				}

				g_pEntityManagerProviderAgent->PushSpawnQueueOld(pSubValues, hSpawnGroup, pWarnings);
			}
			else
			{
				pWarnings->PushFormat("Unknown \"%s\" section in \"%s\"", pszSection, pWorldValues->GetName());
			}
		}
	}
	else if(psError)
	{
		snprintf(psError, nMaxLength, "Can't to load KeyValues from \"%s\" file", sConfigFile);
	}

	pWorldValues->Clear();

	return bResult;
}
