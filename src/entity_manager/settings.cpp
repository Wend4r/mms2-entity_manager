#include "settings.h"

#include <tier0/dbg.h>
#include <tier0/platform.h>
#include <tier1/KeyValues.h>
#include <tier1/utlstring.h> // For filesystem.h
#include <filesystem.h>

#define ENTITY_MANAGER_MAP_CONFIG_FILE "entities.vdf"

extern IFileSystem *filesystem;

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
	                                           "%s\\%s\\" ENTITY_MANAGER_MAP_CONFIG_FILE,
#else
	                                           "%s/%s/" ENTITY_MANAGER_MAP_CONFIG_FILE,
#endif
	                                           pszBasePath, pszMapName);

	KeyValues *pKVEntities = this->m_pEntities;

	bool bResult = pKVEntities->LoadFromFile(filesystem, (const char *)sConfigFile);

	if(bResult)
	{
		Msg("EntityManagerSpace::Settings::Load(): config file is \"%s\" (key value is %p)\n", sConfigFile, pKVEntities);
	}
	else if(psError)
	{
		snprintf(psError, nMaxLength, "Can't to load KeyValue from \"%s\" file\n");
	}

	return bResult;
}

void EntityManagerSpace::Settings::Destroy()
{
	delete this->m_pEntities;
	this->m_pEntities = nullptr;
}
