#include <stdio.h>

#include <tier0/dbg.h>
#include <tier0/platform.h>
#include <tier1/KeyValues.h>
#include <tier1/utlstring.h> // For filesystem.h
#include <filesystem.h>

#include "placement.h"
#include "settings.h"

#define ENTITY_MANAGER_MAP_CONFIG_DIR "configs/maps"
#define ENTITY_MANAGER_MAP_CONFIG_FILE "entities.vdf"

extern IFileSystem *filesystem;

extern EntityManagerSpace::Placement *g_pEntityManagerPlacement;

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

		bResult = this->InternalLoad(pKVEntities, psError, nMaxLength);
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

bool EntityManagerSpace::Settings::InternalLoad(const KeyValues *pEntities, char *psError, size_t nMaxLength)
{
	FOR_EACH_SUBKEY(pEntities, pEntity)
	{
		Msg("Detect \"%s\" classname\n", pEntity->GetName());
	}

	return true;
}
