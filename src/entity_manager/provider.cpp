#include <entity_manager/provider_agent.hpp>
#include <entity_manager/provider.hpp>

#include <filesystem.h>
#include <tier0/strtools.h>
#include <entity2/entitysystem.h>

#include <gamedata.hpp>
#include <any_config.hpp>

#define GAMECONFIG_FOLDER_DIR "gamedata"
#define GAMECONFIG_ENTITYRESOURCEMANIFEST_FILENAME "entityresourcemanifest.games.*"
#define GAMECONFIG_ENTITYSYSTEM_FILENAME "entitysystem.games.*"
#define GAMECONFIG_GAMERESOURCE_FILENAME "gameresource.games.*"
#define GAMECONFIG_GAMESYSTEM_FILENAME "gamesystem.games.*"
#define GAMECONFIG_SOURCE2SERVER_FILENAME "source2server.games.*"
#define GAMECONFIG_SPAWNGROUP_FILENAME "spawngroup.games.*"

extern IFileSystem *filesystem;
extern IVEngineServer *engine;
extern IServerGameDLL *server;

DynLibUtils::CModule g_aLibEngine, 
                     g_aLibServer;

EntityManager::Provider::Provider()
 :  m_mapLibraries(DefLessFunc(const CUtlSymbolLarge))
{
}

bool EntityManager::Provider::Init(GameData::CBufferStringVector &vecMessages)
{
	// Enigne 2.
	{
		const char szEngineModuleName[] = "engine2";

		if(!g_aLibEngine.InitFromMemory(engine))
		{
			static const char *s_pszMessageConcat[] = {"Failed to ", "get ", szEngineModuleName, " module"};

			vecMessages.AddToTail({s_pszMessageConcat});
		}

		m_mapLibraries.Insert(GetSymbol(szEngineModuleName), &g_aLibEngine);
	}

	// Server.
	{
		const char szServerModuleName[] = "server";

		if(!g_aLibServer.InitFromMemory(server))
		{
			static const char *s_pszMessageConcat[] = {"Failed to ", "get ", szServerModuleName, " module"};

			vecMessages.AddToTail({s_pszMessageConcat});
		}

		m_mapLibraries.Insert(GetSymbol(szServerModuleName), &g_aLibServer);
	}

	return true;
}

bool EntityManager::Provider::Load(const char *pszBaseDir, GameData::CBufferStringVector &vecMessages)
{
	bool bResult = LoadGameData(pszBaseDir, vecMessages);

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

const DynLibUtils::CModule *EntityManager::Provider::FindLibrary(const char *pszName) const
{
	auto iFoundIndex = m_mapLibraries.Find(FindSymbol(pszName));

	Assert(IS_VALID_GAMEDATA_INDEX(m_mapLibraries, iFoundIndex));

	return m_mapLibraries.Element(iFoundIndex);
}

CUtlSymbolLarge EntityManager::Provider::GetSymbol(const char *pszText)
{
	return m_aSymbolTable.AddString(pszText);
}

CUtlSymbolLarge EntityManager::Provider::FindSymbol(const char *pszText) const
{
	return m_aSymbolTable.Find(pszText);
}

bool EntityManager::Provider::LoadGameData(const char *pszBaseDir, GameData::CBufferStringVector &vecMessages)
{
	char sBaseConfigDir[MAX_PATH];

	snprintf((char *)sBaseConfigDir, sizeof(sBaseConfigDir), "%s" CORRECT_PATH_SEPARATOR_S "%s", pszBaseDir, GAMECONFIG_FOLDER_DIR);

	return m_aStorage.Load(this, sBaseConfigDir, vecMessages);
}

bool EntityManager::Provider::GameDataStorage::Load(IGameData *pRoot, const char *pszBaseConfigDir, GameData::CBufferStringVector &vecMessages)
{
	bool bResult = true;

	struct
	{
		const char *pszFilename;
		bool (EntityManager::Provider::GameDataStorage::*pfnLoad)(IGameData *, KeyValues3 *, GameData::CBufferStringVector &);
	} aConfigs[] =
	{
		{
			GAMECONFIG_ENTITYRESOURCEMANIFEST_FILENAME,
			&GameDataStorage::LoadEntityResourceManifest
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

	char sConfigFile[MAX_PATH];

	const char *pszConfigPathID = "GAME";

	CUtlString sError;

	AnyConfig::CLoadFromFile_General aLoadPresets({{&sError, NULL, pszConfigPathID}, g_KV3Format_Generic});

	for(size_t n = 0, nSize = std::size(aConfigs); n < nSize; n++)
	{
		AnyConfig::Anyone aGameConfig;

		snprintf((char *)sConfigFile, sizeof(sConfigFile), "%s" CORRECT_PATH_SEPARATOR_S "%s", pszBaseConfigDir, aConfigs[n].pszFilename);

		CUtlVector<CUtlString> vecConfigFiles;

		filesystem->FindFileAbsoluteList(vecConfigFiles, (const char *)sConfigFile, pszConfigPathID);

		bResult = vecConfigFiles.Count() > 0;

		if(bResult)
		{
			aLoadPresets.m_pszFilename = vecConfigFiles[0].Get();
			bResult = aGameConfig.Load(aLoadPresets);

			if(bResult)
			{
				(this->*(aConfigs[n].pfnLoad))(pRoot, aGameConfig.Get(), vecMessages);
			}
			else
			{
				const char *pszMessageConcat[] = {"Failed to ", "load \"", sConfigFile, "\" file", ": ", sError.Get()};

				vecMessages.AddToTail({pszMessageConcat});
			}
		}
		else
		{
			const char *pszMessageConcat[] = {"Failed to ", "find \"", sConfigFile, "\" file", ": ", sError.Get()};

			vecMessages.AddToTail({pszMessageConcat});
		}
	}

	return bResult;
}

bool EntityManager::Provider::GameDataStorage::LoadEntityResourceManifest(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return m_aEntityResourceManifest.Load(pRoot, pGameConfig, vecMessages);
}

bool EntityManager::Provider::GameDataStorage::LoadEntitySystem(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return m_aEntitySystem.Load(pRoot, pGameConfig, vecMessages);
}

bool EntityManager::Provider::GameDataStorage::LoadGameResource(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return m_aGameResource.Load(pRoot, pGameConfig, vecMessages);
}

bool EntityManager::Provider::GameDataStorage::LoadGameSystem(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return m_aGameSystem.Load(pRoot, pGameConfig, vecMessages);
}

bool EntityManager::Provider::GameDataStorage::LoadSource2Server(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return m_aSource2Server.Load(pRoot, pGameConfig, vecMessages);
}

bool EntityManager::Provider::GameDataStorage::LoadEntitySpawnGroup(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CBufferStringVector &vecMessages)
{
	return m_aSpawnGroup.Load(pRoot, pGameConfig, vecMessages);
}

const EntityManager::Provider::GameDataStorage::EntityResourceManifest &EntityManager::Provider::GameDataStorage::GetEntityResourceManifest() const
{
	return m_aEntityResourceManifest;
}

const EntityManager::Provider::GameDataStorage::EntitySystem &EntityManager::Provider::GameDataStorage::GetEntitySystem() const
{
	return m_aEntitySystem;
}

const EntityManager::Provider::GameDataStorage::GameResource &EntityManager::Provider::GameDataStorage::GetGameResource() const
{
	return m_aGameResource;
}

const EntityManager::Provider::GameDataStorage::GameSystem &EntityManager::Provider::GameDataStorage::GetGameSystem() const
{
	return m_aGameSystem;
}

const EntityManager::Provider::GameDataStorage::Source2Server &EntityManager::Provider::GameDataStorage::GetSource2Server() const
{
	return m_aSource2Server;
}

const EntityManager::Provider::GameDataStorage::SpawnGroup &EntityManager::Provider::GameDataStorage::GetSpawnGroup() const
{
	return m_aSpawnGroup;
}

const EntityManager::Provider::GameDataStorage &EntityManager::Provider::GetGameDataStorage() const
{
	return m_aStorage;
}
