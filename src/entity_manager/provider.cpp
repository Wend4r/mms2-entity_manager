#include "provider.hpp"

#include "gamedata.hpp"
#include "provider_agent.hpp"

#include <filesystem.h>
#include <tier0/strtools.h>
#include <entity2/entitysystem.h>

#define GAMECONFIG_FOLDER_DIR "gamedata"
#define GAMECONFIG_ENTITYSYSTEM_FILENAME "entitysystem.games.txt"
#define GAMECONFIG_GAMERESOURCE_FILENAME "gameresource.games.txt"
#define GAMECONFIG_GAMESYSTEM_FILENAME "gamesystem.games.txt"
#define GAMECONFIG_SOURCE2SERVER_FILENAME "source2server.games.txt"
#define GAMECONFIG_SPAWNGROUP_FILENAME "spawngroup.games.txt"

DLL_IMPORT EntityManager::ProviderAgent *g_pEntityManagerProviderAgent;

DLL_IMPORT IFileSystem *filesystem;

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

	snprintf((char *)sBaseConfigDir, sizeof(sBaseConfigDir), "%s" CORRECT_PATH_SEPARATOR_S "%s", pszBaseDir, GAMECONFIG_FOLDER_DIR);

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
			GAMECONFIG_ENTITYSYSTEM_FILENAME,
			&GameDataStorage::LoadEntitySystem
		},
		{
			GAMECONFIG_GAMERESOURCE_FILENAME,
			&GameDataStorage::LoadGameResource
		},
		{
			GAMECONFIG_GAMESYSTEM_FILENAME,
			&GameDataStorage::LoadGameSystem
		},
		{
			GAMECONFIG_SOURCE2SERVER_FILENAME,
			&GameDataStorage::LoadSource2Server
		},
		{
			GAMECONFIG_SPAWNGROUP_FILENAME,
			&GameDataStorage::LoadEntitySpawnGroup
		}
	};

	for(size_t n = 0, nSize = std::size(aConfigs); n < nSize; n++)
	{
		snprintf((char *)sConfigFile, sizeof(sConfigFile), "%s" CORRECT_PATH_SEPARATOR_S "%s", pszBaseConfigDir, aConfigs[n].pszFilename);

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

bool EntityManager::Provider::GameDataStorage::LoadEntitySystem(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aEntitySystem.Load(pRoot, pGameConfig, psError, nMaxLength);
}

bool EntityManager::Provider::GameDataStorage::LoadGameResource(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aGameResource.Load(pRoot, pGameConfig, psError, nMaxLength);
}

bool EntityManager::Provider::GameDataStorage::LoadGameSystem(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aGameSystem.Load(pRoot, pGameConfig, psError, nMaxLength);
}

bool EntityManager::Provider::GameDataStorage::LoadSource2Server(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aSource2Server.Load(pRoot, pGameConfig, psError, nMaxLength);
}

bool EntityManager::Provider::GameDataStorage::LoadEntitySpawnGroup(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aSpawnGroup.Load(pRoot, pGameConfig, psError, nMaxLength);
}

const EntityManager::Provider::GameDataStorage::EntitySystem &EntityManager::Provider::GameDataStorage::GetEntitySystem() const
{
	return this->m_aEntitySystem;
}

const EntityManager::Provider::GameDataStorage::GameResource &EntityManager::Provider::GameDataStorage::GetGameResource() const
{
	return this->m_aGameResource;
}

const EntityManager::Provider::GameDataStorage::GameSystem &EntityManager::Provider::GameDataStorage::GetGameSystem() const
{
	return this->m_aGameSystem;
}

const EntityManager::Provider::GameDataStorage::Source2Server &EntityManager::Provider::GameDataStorage::GetSource2Server() const
{
	return this->m_aSource2Server;
}

const EntityManager::Provider::GameDataStorage::SpawnGroup &EntityManager::Provider::GameDataStorage::GetSpawnGroup() const
{
	return this->m_aSpawnGroup;
}

const EntityManager::Provider::GameDataStorage &EntityManager::Provider::GetGameDataStorage() const
{
	return this->m_aStorage;
}
