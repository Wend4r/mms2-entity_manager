#include <stdio.h>

#include "settings.h"

#include <tier0/dbg.h>
#include <tier0/platform.h>
#include <tier1/KeyValues.h>
#include <tier1/utlstring.h>
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

	snprintf((char *)sBaseConfigsDir, sizeof(sBaseConfigsDir), 
#ifdef PLATFORM_WINDOWS
		"%s\\%s\\%s\\%s",
#else
		"%s/%s/%s/%s",
#endif
		pszBasePath, ENTITY_MANAGER_MAP_CONFIG_DIR, ENTITY_MANAGER_MAP_CONFIG_SPAWNGROUPS_DIR, pszSpawnGroupName);

	bool bResult = this->LoadWorld(hSpawnGroup, (const char *)sBaseConfigsDir, psError, nMaxLength, pDetails, pWarnings);

	if(bResult)
	{
		// ...
	}

	return bResult;
}

void EntityManager::Settings::Clear()
{
	this->m_pWorld->Clear();
}

void EntityManager::Settings::Destroy()
{
	delete this->m_pWorld;
	this->m_pWorld = nullptr;
}

bool EntityManager::Settings::LoadWorld(SpawnGroupHandle_t hSpawnGroup, const char *pszBaseConfigsDir, char *psError, size_t nMaxLength, Logger::Scope *pDetails, Logger::Scope *pWarnings)
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
		FOR_EACH_SUBKEY(pWorldValues, pEntityValues)
		{
			g_pEntityManagerProviderAgent->PushSpawnQueueOld(pEntityValues, hSpawnGroup, pDetails, pWarnings);
		}
	}
	else if(psError)
	{
		snprintf(psError, nMaxLength, "Can't to load KeyValues from \"%s\" file", sConfigFile);
	}

	return bResult;
}
