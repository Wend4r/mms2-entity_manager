#include <stdio.h>

#include "settings.h"

#include <tier0/dbg.h>
#include <tier0/platform.h>
#include <tier1/KeyValues.h>
#include <tier1/utlstring.h>
#include <tier1/utlvector.h>
#include <filesystem.h>

#include "provider_agent.h"

#define ENTITY_MANAGER_MAP_CONFIG_DIR "configs/maps"
#define ENTITY_MANAGER_MAP_CONFIG_WORLD_FILE "world.vdf"

extern IFileSystem *filesystem;

extern EntityManagerSpace::ProviderAgent *g_pEntityManagerProviderAgent;

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
		FOR_EACH_SUBKEY(pWorldValues, pEntityValues)
		{
			g_pEntityManagerProviderAgent->PushSpawnQueue(pEntityValues);
		}
	}
	else if(psError)
	{
		snprintf(psError, nMaxLength, "Can't to load KeyValue from \"%s\" file", sConfigFile);
	}

	return bResult;
}
