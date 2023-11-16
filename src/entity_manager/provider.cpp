#include "provider.h"

#include "gamedata.h"
#include "provider_agent.h"

#include <filesystem.h>
#include <entity2/entitysystem.h>

#define GAMECONFIG_FOLDER_DIR "gamedata"
#define GAMECONFIG_ENTITYKEYVALUES_FILENAME "entitykeyvalues.games.txt"
#define GAMECONFIG_ENTITYSYSTEM_FILENAME "entitysystem.games.txt"
#define GAMECONFIG_GAMERESOURCE_FILENAME "gameresource.games.txt"
#define GAMECONFIG_SPAWNGROUP_FILENAME "spawngroup.games.txt"

extern EntityManager::ProviderAgent *g_pEntityManagerProviderAgent;

extern IFileSystem *filesystem;

bool EntityManager::Provider::Init(char *psError, size_t nMaxLength)
{
	char sGameDataError[256];

	bool bResult = this->m_aData.Init((char *)sGameDataError, sizeof(sGameDataError));

	if(!bResult && psError)
	{
		snprintf(psError, nMaxLength, "Failed to init a gamedata: %s", sGameDataError);
	}

	return bResult;
}

bool EntityManager::Provider::Load(const char *pszBaseDir, char *psError, size_t nMaxLength)
{
	bool bResult = this->LoadGameData(pszBaseDir, psError, nMaxLength);

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

bool EntityManager::Provider::LoadGameData(const char *pszBaseDir, char *psError, size_t nMaxLength)
{
	char sBaseConfigDir[MAX_PATH];

	snprintf((char *)sBaseConfigDir, sizeof(sBaseConfigDir), 
#ifdef PLATFORM_WINDOWS
		"%s\\%s", 
#else
		"%s/%s", 
#endif
		pszBaseDir, GAMECONFIG_FOLDER_DIR);

	char sGameDataError[256];

	bool bResult = this->m_aStorage.Load((IGameData *)&this->m_aData, (const char *)sBaseConfigDir, (char *)sGameDataError, sizeof(sGameDataError));

	if(!bResult && psError)
	{
		snprintf(psError, nMaxLength, "Failed to load a gamedata: %s", sGameDataError);
	}

	return bResult;
}

bool EntityManager::Provider::GameDataStorage::Load(IGameData *pRoot, const char *pszBaseConfigDir, char *psError, size_t nMaxLength)
{
	KeyValues *pGameConfig = new KeyValues("Games");

	char sConfigFile[MAX_PATH];

	bool bResult = true;

	struct
	{
		const char *pszFilename;
		bool (EntityManager::Provider::GameDataStorage::*pfnLoad)(IGameData *, KeyValues *, char *, size_t);
	} aConfigs[] =
	{
		{
			GAMECONFIG_ENTITYKEYVALUES_FILENAME,
			&GameDataStorage::LoadEntityKeyValues
		},
		{
			GAMECONFIG_ENTITYSYSTEM_FILENAME,
			&GameDataStorage::LoadEntitySystem
		},
		{
			GAMECONFIG_GAMERESOURCE_FILENAME,
			&GameDataStorage::LoadGameResource
		},
		{
			GAMECONFIG_SPAWNGROUP_FILENAME,
			&GameDataStorage::LoadEntitySpawnGroup
		}
	};

	for(size_t n = 0, nSize = std::size(aConfigs); n < nSize; n++)
	{
		snprintf((char *)sConfigFile, sizeof(sConfigFile), 
#ifdef PLATFORM_WINDOWS
			"%s\\%s",
#else
			"%s/%s", 
#endif
			pszBaseConfigDir, aConfigs[n].pszFilename);

		bResult = pGameConfig->LoadFromFile(filesystem, (const char *)sConfigFile);

		if(bResult)
		{
			bResult = (this->*(aConfigs[n].pfnLoad))(pRoot, pGameConfig, psError, nMaxLength);

			if(bResult)
			{
				pGameConfig->Clear();
			}
			else
			{
				break;
			}
		}
		else if(psError)
		{
			snprintf(psError, nMaxLength, "Failed to load KeyValues from \"%s\" file", sConfigFile);

			break;
		}
	}

	delete pGameConfig;

	return bResult;
}

bool EntityManager::Provider::GameDataStorage::LoadEntityKeyValues(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aEntityKeyValues.Load(pRoot, pGameConfig, psError, nMaxLength);
}

bool EntityManager::Provider::GameDataStorage::LoadEntitySystem(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aEntitySystem.Load(pRoot, pGameConfig, psError, nMaxLength);
}

bool EntityManager::Provider::GameDataStorage::LoadGameResource(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aGameResource.Load(pRoot, pGameConfig, psError, nMaxLength);
}

bool EntityManager::Provider::GameDataStorage::LoadEntitySpawnGroup(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aSpawnGroup.Load(pRoot, pGameConfig, psError, nMaxLength);
}

const EntityManager::Provider::GameDataStorage::EntityKeyValues &EntityManager::Provider::GameDataStorage::GetEntityKeyValues() const
{
	return this->m_aEntityKeyValues;
}

const EntityManager::Provider::GameDataStorage::EntitySystem &EntityManager::Provider::GameDataStorage::GetEntitySystem() const
{
	return this->m_aEntitySystem;
}

const EntityManager::Provider::GameDataStorage::GameResource &EntityManager::Provider::GameDataStorage::GetGameResource() const
{
	return this->m_aGameResource;
}

const EntityManager::Provider::GameDataStorage::SpawnGroup &EntityManager::Provider::GameDataStorage::GetSpawnGroup() const
{
	return this->m_aSpawnGroup;
}

const EntityManager::Provider::GameDataStorage &EntityManager::Provider::GetGameDataStorage() const
{
	return this->m_aStorage;
}
