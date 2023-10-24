/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * Metamod:Source Entity Manger
 * Written by Wend4r.
 * ======================================================
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from 
 * the use of this software.
 */

#include <stdio.h>
#include <string>
#include <functional>

// Game SDK.
#include <eiface.h>
#include <iserver.h>
#include <entity2/entitysystem.h>
#include <game_systems/spawngroup_manager.h>
#include <tier0/dbg.h>
#include <tier1/convar.h>

#include "entity_manager.h"

class GameSessionConfiguration_t { };

SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);
SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t &, ISource2WorldSession *, const char *);
SH_DECL_HOOK2_void(CSpawnGroupMgrGameSystem, AllocateSpawnGroup, SH_NOATTRIB, 0, SpawnGroupHandle_t, ISpawnGroup *);
SH_DECL_HOOK4(CSpawnGroupMgrGameSystem, CreateLoadingSpawnGroup, SH_NOATTRIB, 0, ILoadingSpawnGroup *, SpawnGroupHandle_t, bool, bool, const CUtlVector<const CEntityKeyValues *> *);

static EntityManagerPlugin s_aEntityManager;
EntityManagerPlugin *g_pEntityManager = &s_aEntityManager;  // To extern usage.

static EntityManager::GameData s_aEntityManagerGameData;
EntityManager::GameData *g_pEntityManagerGameData = &s_aEntityManagerGameData;

static EntityManager::Logger s_aEntityManagerLogger;
EntityManager::Logger *g_pEntityManagerLogger = &s_aEntityManagerLogger;

static EntityManager::Provider s_aEntityManagerProvider;
EntityManager::Provider *g_pEntityManagerProvider = &s_aEntityManagerProvider;

static EntityManager::ProviderAgent s_aEntityManagerProviderAgent;
EntityManager::ProviderAgent *g_pEntityManagerProviderAgent = &s_aEntityManagerProviderAgent;

IVEngineServer *engine = NULL;
ICvar *icvar = NULL;
IGameEventManager2 *gameevents = NULL;
IFileSystem *filesystem = NULL;
IServerGameDLL *server = NULL;

CGameEntitySystem *g_pGameEntitySystem = NULL;
CSpawnGroupMgrGameSystem *g_pSpawnGroupMgr = NULL;

// Should only be called within the active game loop (i e map should be loaded and active)
// otherwise that'll be nullptr!
CGlobalVars *GetGameGlobals()
{
	INetworkGameServer *server = g_pNetworkServerService->GetIGameServer();

	if(!server)
		return nullptr;

	return g_pNetworkServerService->GetIGameServer()->GetGlobals();
}

PLUGIN_EXPOSE(EntityManagerPlugin, s_aEntityManager);
bool EntityManagerPlugin::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, gameevents, IGameEventManager2, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pGameResourceServiceServer, IGameResourceServiceServer, GAMERESOURCESERVICESERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetFileSystemFactory, filesystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);

	// Currently doesn't work from within mm side, use GetGameGlobals() in the mean time instead
	// gpGlobals = ismm->GetCGlobals();

	META_CONPRINTF( "Starting %s plugin.\n", this->GetName());

	// SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &EntityManagerPlugin::OnGameFrameHook, true);
	SH_ADD_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &EntityManagerPlugin::OnStartupServerHook, true);

	META_CONPRINTF( "All hooks started!\n" );

	g_pCVar = icvar;
	ConVar_Register(FCVAR_RELEASE | FCVAR_GAMEDLL);

	// Initialize and load a gamedata.
	{
		char sGameDataError[1024];

		if(s_aEntityManagerGameData.Init((char *)sGameDataError, sizeof(sGameDataError)))
		{
			if(!this->LoadGameData((char *)sGameDataError, sizeof(sGameDataError)))
			{
				snprintf(error, maxlen, "Failed to load a gamedata: %s", sGameDataError);

				return false;
			}
		}
		else
		{
			snprintf(error, maxlen, "Failed to init a gamedata: %s", sGameDataError);

			return false;
		}
	}

	// Initialize and load a provider.
	{
		char sProviderError[256];

		if(s_aEntityManagerProvider.Init((char *)sProviderError, sizeof(sProviderError)))
		{
			if(!this->LoadProvider((char *)sProviderError, sizeof(sProviderError)))
			{
				snprintf(error, maxlen, "Failed to load a provider: %s", sProviderError);

				return false;
			}
		}
		else
		{
			snprintf(error, maxlen, "Failed to init a provider: %s", sProviderError);

			return false;
		}
	}

	// Initialize a provider agent.
	{
		char sProviderAgentError[256];

		if(!s_aEntityManagerProviderAgent.Init((char *)sProviderAgentError, sizeof(sProviderAgentError)))
		{
			snprintf(error, maxlen, "Failed to init a provider agent: %s", sProviderAgentError);

			return false;
		}
	}

	// Initialize a settings.
	{
		char sSettingsError[256];

		if(!this->m_aSettings.Init(sSettingsError, sizeof(sSettingsError)))
		{
			snprintf(error, maxlen, "Failed to init a settings: %s", sSettingsError);

			return false;
		}
	}

	if(late)
	{
		INetworkGameServer *pNewServer = g_pNetworkServerService->GetIGameServer();

		if(pNewServer)
		{
			this->InitEntitySystem();
			this->InitSpawnGroup();

			// Load by spawn groups now.
			{
				auto pSpawnGroupMgr = (EntityManager::CSpawnGroupMgrGameSystemProvider *)g_pSpawnGroupMgr;

				auto aWarnings = s_aEntityManagerLogger.CreateWarningsScope();

				pSpawnGroupMgr->WhileBySpawnGroups([this, &aWarnings](SpawnGroupHandle_t h, CMapSpawnGroup *pSpawnGroup) -> void {
					char sSettingsError[256];

					if(this->LoadSettings(h, pSpawnGroup->GetSpawnGroupName(), (char *)sSettingsError, sizeof(sSettingsError)))
					{
						CUtlVector<const CEntityKeyValues *> vecKeyValues;

						ILoadingSpawnGroup *pLoadingSpawnGroup = g_pSpawnGroupMgr->CreateLoadingSpawnGroup(h, false, false, &vecKeyValues);

						pSpawnGroup->SetSpawnGroupLoading(pLoadingSpawnGroup);
					}
					else
					{
						aWarnings.PushFormat("Failed to load a settings: %s", sSettingsError);
					}
				});

				aWarnings.Send([](const char *pszContent) {
					s_aEntityManagerLogger.Warning(pszContent);
				});

				// s_aEntityManagerProviderAgent.SpawnQueued();
			}
		}
	}

	return true;
}

bool EntityManagerPlugin::Unload(char *error, size_t maxlen)
{
	this->m_aSettings.Destroy();
	s_aEntityManagerGameData.Destroy();

	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &EntityManagerPlugin::OnGameFrameHook, true);
	SH_REMOVE_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &EntityManagerPlugin::OnStartupServerHook, true);

	this->DestroyEntitySystem();
	this->DestroySpawnGroup();

	return true;
}

void EntityManagerPlugin::AllPluginsLoaded()
{
	/* This is where we'd do stuff that relies on the mod or other plugins 
	 * being initialized (for example, cvars added and events registered).
	 */
}

void EntityManagerPlugin::InitEntitySystem()
{
	g_pGameEntitySystem = *reinterpret_cast<CGameEntitySystem **>(reinterpret_cast<uintptr_t>(g_pGameResourceServiceServer) + this->m_nGameResourceServiceEntitySystemOffset);
}

void EntityManagerPlugin::DestroyEntitySystem()
{
	// ...
}

void EntityManagerPlugin::InitSpawnGroup()
{
	g_pSpawnGroupMgr = *this->m_ppSpawnGroupMgrAddress;

	SH_ADD_HOOK_MEMFUNC(CSpawnGroupMgrGameSystem, AllocateSpawnGroup, g_pSpawnGroupMgr, this, &EntityManagerPlugin::OnAllocateSpawnGroupHook, true);
	SH_ADD_HOOK_MEMFUNC(CSpawnGroupMgrGameSystem, CreateLoadingSpawnGroup, g_pSpawnGroupMgr, this, &EntityManagerPlugin::OnCreateLoadingSpawnGroupHook, false);
}

void EntityManagerPlugin::DestroySpawnGroup()
{
	SH_REMOVE_HOOK_MEMFUNC(CSpawnGroupMgrGameSystem, AllocateSpawnGroup, g_pSpawnGroupMgr, this, &EntityManagerPlugin::OnAllocateSpawnGroupHook, true);
	SH_REMOVE_HOOK_MEMFUNC(CSpawnGroupMgrGameSystem, CreateLoadingSpawnGroup, g_pSpawnGroupMgr, this, &EntityManagerPlugin::OnCreateLoadingSpawnGroupHook, false);
}

bool EntityManagerPlugin::LoadGameData(char *psError, size_t nMaxLength)
{
	s_aEntityManagerGameData.Clear();

	bool bResult = s_aEntityManagerGameData.Load(this->m_sBasePath.c_str(), psError, nMaxLength);

	// Load offsets.
	if(bResult)
	{
		const char *pszOffsetName = "CGameResourceService::m_pEntitySystem";

		ptrdiff_t nOffset = s_aEntityManagerGameData.GetEntitySystemOffset(pszOffsetName);

		bResult = nOffset != -1;

		if(bResult)
		{
			this->m_nGameResourceServiceEntitySystemOffset = nOffset;
		}
		else if(psError)
		{
			snprintf(psError, nMaxLength, "Failed to get \"%s\" offset", pszOffsetName);
		}
	}

	// Load addresses.
	if(bResult)
	{
		const char *pszAddressName = "&g_pSpawnGroupMgr";

		CMemory pAddress = s_aEntityManagerGameData.GetSpawnGroupAddress(pszAddressName);

		bResult = (bool)pAddress;

		if(bResult)
		{
			this->m_ppSpawnGroupMgrAddress = (CSpawnGroupMgrGameSystem **)pAddress.GetPtr();
		}
		else if(psError)
		{
			snprintf(psError, nMaxLength, "Failed to get \"%s\" address", pszAddressName);
		}
	}

	return bResult;
}

bool EntityManagerPlugin::LoadProvider(char *psError, size_t nMaxLength)
{
	char sProviderError[256];

	bool bResult = s_aEntityManagerProvider.Load((char *)sProviderError, sizeof(sProviderError));

	if(!bResult && psError)
	{
		snprintf(psError, nMaxLength, "Failed to init a provider: %s", sProviderError);
	}

	return bResult;
}

bool EntityManagerPlugin::LoadSettings(SpawnGroupHandle_t hSpawnGroup, const char *pszSpawnGroupName, char *psError, size_t nMaxLength)
{
	return this->m_aSettings.Load(hSpawnGroup, this->m_sBasePath.c_str(), pszSpawnGroupName, psError, nMaxLength);
}

void EntityManagerPlugin::OnBasePathChanged(const char *pszNewOne)
{
	this->m_sBasePath = pszNewOne;

	// Load a gamedata.
	{
		char sGameDataError[1024];

		if(!this->LoadGameData((char *)sGameDataError, sizeof(sGameDataError)))
		{
			s_aEntityManagerLogger.ErrorFormat("Failed to load a gamedata: %s\n", sGameDataError);
		}
	}

	// Load a provider.
	{
		char sProviderError[256];

		if(!this->LoadProvider((char *)sProviderError, sizeof(sProviderError)))
		{
			s_aEntityManagerLogger.WarningFormat("Failed to load a provider: %s\n", sProviderError);
		}
	}

	// Load a settings.
	{
		// char sSettingsError[256];

		// if(!this->LoadSettings(this->m_sCurrentMap.c_str(), (char *)sSettingsError, sizeof(sSettingsError)))
		// {
		// 	Warning("Failed to load a settings: %s\n", sSettingsError);
		// }
	}
}

void EntityManagerPlugin::OnSetBasePathCommand(const CCommandContext &context, const CCommand &args)
{
	if(args.ArgC() != 2)
	{
		Msg("Usage: %s <base path>\n", args[0]);

		return;
	}

	this->OnBasePathChanged(args[1]);
	Msg("Base path is \"%s\"\n", this->m_sBasePath.c_str());
}

void EntityManagerPlugin::OnGameFrameHook( bool simulating, bool bFirstTick, bool bLastTick )
{
	/**
	 * simulating:
	 * ***********
	 * true  | game is ticking
	 * false | game is not ticking
	 */
}

void EntityManagerPlugin::OnStartupServerHook(const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession, const char *)
{
	INetworkGameServer *pNetServer = g_pNetworkServerService->GetIGameServer();

	if(pNetServer)
	{
		const char *pszMapName = pNetServer->GetMapName();

		if(g_pGameEntitySystem)
		{
			this->DestroyEntitySystem();
		}

		if(g_pSpawnGroupMgr)
		{
			this->DestroySpawnGroup();
		}

		this->InitEntitySystem();
		this->InitSpawnGroup();
	}
	else
	{
		s_aEntityManagerLogger.Warning("Failed to get a net server\n");
	}
}

void EntityManagerPlugin::OnAllocateSpawnGroupHook(SpawnGroupHandle_t handle, ISpawnGroup *pSpawnGroup)
{
	s_aEntityManagerLogger.MessageFormat("EntityManagerPlugin::OnAllocateSpawnGroupHook(%d, %s)\n", handle, pSpawnGroup->GetName());

	// Load settings by spawn group name
	{
		char sSettingsError[256];

		if(!this->LoadSettings(handle, pSpawnGroup->GetName(), (char *)sSettingsError, sizeof(sSettingsError)))
		{
			s_aEntityManagerLogger.WarningFormat("Failed to load a settings: %s\n", sSettingsError);
		}
	}
}

ILoadingSpawnGroup *EntityManagerPlugin::OnCreateLoadingSpawnGroupHook(SpawnGroupHandle_t handle, bool bSynchronouslySpawnEntities, bool bConfirmResourcesLoaded, const CUtlVector<const CEntityKeyValues *> *pKeyValues)
{
	s_aEntityManagerLogger.MessageFormat("EntityManagerPlugin::CreateLoadingSpawnGroup(%d, bSynchronouslySpawnEntities = %s, bConfirmResourcesLoaded = %s, pKeyValues = %p)\n", handle, bSynchronouslySpawnEntities ? "true" : "false", bConfirmResourcesLoaded ? "true" : "false", pKeyValues);

	s_aEntityManagerProviderAgent.AddSpawnQueuedToTail(const_cast<CUtlVector<const CEntityKeyValues *> *>(pKeyValues), handle);
	s_aEntityManagerProviderAgent.ReleaseSpawnQueued(handle);

	RETURN_META_VALUE_NEWPARAMS(MRES_HANDLED, nullptr, &CSpawnGroupMgrGameSystem::CreateLoadingSpawnGroup, (handle, bSynchronouslySpawnEntities, bConfirmResourcesLoaded, pKeyValues));
}

bool EntityManagerPlugin::Pause(char *error, size_t maxlen)
{
	return true;
}

bool EntityManagerPlugin::Unpause(char *error, size_t maxlen)
{
	return true;
}

const char *EntityManagerPlugin::GetLicense()
{
	return "Public Domain";
}

const char *EntityManagerPlugin::GetVersion()
{
	return "1.0.0";
}

const char *EntityManagerPlugin::GetDate()
{
	return __DATE__;
}

const char *EntityManagerPlugin::GetLogTag()
{
	return "ENTITY_MANAGER";
}

const char *EntityManagerPlugin::GetAuthor()
{
	return "Wend4r";
}

const char *EntityManagerPlugin::GetDescription()
{
	return "Plugin to manage a entities";
}

const char *EntityManagerPlugin::GetName()
{
	return "Entity Manager";
}

const char *EntityManagerPlugin::GetURL()
{
	return "https://github.com/mms-entity_manager";
}
