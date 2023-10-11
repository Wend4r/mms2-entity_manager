#include <stdio.h>

#include <tier0/dbg.h>
#include <tier0/platform.h>
#include <tier1/KeyValues.h>
#include <tier1/utlstring.h> // For filesystem.h
#include <filesystem.h>

#include "placement/entitysystem_provider.h"
#include "settings.h"

#define ENTITY_MANAGER_MAP_CONFIG_DIR "configs/maps"
#define ENTITY_MANAGER_MAP_CONFIG_FILE "entities.vdf"

extern IFileSystem *filesystem;

extern CGameEntitySystem *g_pEntitySystem;

bool EntityManagerSpace::Settings::Init(char *psError, size_t nMaxLength)
{
	this->m_pEntities = new KeyValues("Entities");

	return true;
}

bool EntityManagerSpace::Settings::Load(const char *pszBasePath, const char *pszMapName, char *psError, size_t nMaxLength)
{
	char sConfigFile[MAX_PATH];

	snprintf(sConfigFile, sizeof(sConfigFile), 
#ifdef PLATFORM_WINDOWS
	                                           "%s\\%s\\%s\\%s",
#else
	                                           "%s/%s/%s/%s",
#endif
	                                           pszBasePath, ENTITY_MANAGER_MAP_CONFIG_DIR, pszMapName, ENTITY_MANAGER_MAP_CONFIG_FILE);

	KeyValues *pKVEntities = this->m_pEntities;

	bool bResult = pKVEntities->LoadFromFile(filesystem, (const char *)sConfigFile);

	if(bResult)
	{
		DevMsg("EntityManagerSpace::Settings::Load(): config file is \"%s\" (key value is %p)\n", sConfigFile, pKVEntities);

		bResult = this->LoadAndCreateEntities(pKVEntities, psError, nMaxLength);
	}
	else if(psError)
	{
		snprintf(psError, nMaxLength, "Can't to load KeyValue from \"%s\" file", sConfigFile);
	}

	return bResult;
}

void EntityManagerSpace::Settings::Clear()
{
	this->m_pEntities->Clear();
}

void EntityManagerSpace::Settings::Destroy()
{
	delete this->m_pEntities;
	this->m_pEntities = nullptr;
}

bool EntityManagerSpace::Settings::LoadAndCreateEntities(const KeyValues *pEntityValues, char *psError, size_t nMaxLength)
{
	FOR_EACH_SUBKEY(pEntityValues, pEntitySection)
	{
		((CEntitySystemProvider *)g_pEntitySystem)->CreateEntity(CEntityIndex(-1), pEntitySection->GetName(), ENTITY_NETWORKING_MODE_DEFAULT, (SpawnGroupHandle_t)-1, -1, false);
	}

	return true;
}
