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

// Game SDK.
#include <entity2/entitysystem.h>
#include <tier0/dbg.h>
#include <tier1/convar.h>
#include <eiface.h>
#include <iserver.h>

#include "entity_manager.h"

class GameSessionConfiguration_t { };

SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);
SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t &, ISource2WorldSession *, const char *);

EntityManager g_aEntityManager;

IVEngineServer *engine = NULL;
ICvar *icvar = NULL;
IGameEventManager2 *gameevents = NULL;
IFileSystem *filesystem = NULL;
IServerGameDLL *server = NULL;

CEntitySystem *g_pEntitySystem;

// Should only be called within the active game loop (i e map should be loaded and active)
// otherwise that'll be nullptr!
CGlobalVars *GetGameGlobals()
{
	INetworkGameServer *server = g_pNetworkServerService->GetIGameServer();

	if(!server)
		return nullptr;

	return g_pNetworkServerService->GetIGameServer()->GetGlobals();
}

PLUGIN_EXPOSE(EntityManager, g_aEntityManager);
bool EntityManager::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
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

	// SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &EntityManager::OnGameFrameHook, true);
	SH_ADD_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &EntityManager::OnStartupServerHook, true);

	META_CONPRINTF( "All hooks started!\n" );

	g_pCVar = icvar;
	ConVar_Register(FCVAR_RELEASE | FCVAR_GAMEDLL);

	bool bResult = true;

	{
		char sSettingsError[256];

		bResult = this->m_aSettings.Init(sSettingsError, sizeof(sSettingsError));

		if(!bResult)
		{
			snprintf(error, maxlen, "Failed to init a settings: %s", sSettingsError);
		}
	}


	if(late)
	{
		INetworkGameServer *pServer = g_pNetworkServerService->GetIGameServer();

		if(pServer)
		{
			const char *pszMapName = pServer->GetMapName();

			g_aEntityManager.OnLevelInit(pszMapName, nullptr, this->m_sOldMap.c_str(), nullptr, false, false);
			this->m_sOldMap = std::string(pszMapName);
		}
	}

	return bResult;
}

bool EntityManager::Unload(char *error, size_t maxlen)
{
	this->m_aSettings.Destroy();

	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &EntityManager::OnGameFrameHook, true);
	SH_REMOVE_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &EntityManager::OnStartupServerHook, true);

	return true;
}

void EntityManager::AllPluginsLoaded()
{
	/* This is where we'd do stuff that relies on the mod or other plugins 
	 * being initialized (for example, cvars added and events registered).
	 */
}

void EntityManager::OnSetBasePathCommand(const CCommandContext &context, const CCommand &args)
{
	if(args.ArgC() != 2)
	{
		Msg("Usage: %s <base path>\n", args[0]);

		return;
	}

	this->m_sBasePath = std::string(args[1]);

	Msg("Base path is \"%s\"\n", this->m_sBasePath.c_str());
}

// Potentially might not work
void EntityManager::OnLevelInit(char const *pszMapName,
                                char const *pszMapEntities,
                                char const *pszOldLevel,
                                char const *pszLandmarkName,
                                bool bIsLoadGame,
                                bool bIsBackground)
{
	this->m_aSettings.Clear();

	{
		char sBuffer[256];

		if(!this->m_aSettings.Load(this->m_sBasePath.c_str(), pszMapName, (char *)sBuffer, sizeof(sBuffer)))
		{
			Warning("Failed to load settings: %s\n", sBuffer);
		}
	}
}

void EntityManager::OnGameFrameHook( bool simulating, bool bFirstTick, bool bLastTick )
{
	/**
	 * simulating:
	 * ***********
	 * true  | game is ticking
	 * false | game is not ticking
	 */
}

void EntityManager::OnStartupServerHook(const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession, const char *pszMapName)
{
	g_pEntitySystem = *reinterpret_cast<CEntitySystem **>(reinterpret_cast<uintptr_t>(g_pGameResourceServiceServer) 
#if defined(_WINDOWS) && defined(X64BITS)
	+ 0x58
#elif defined(_LINUX) && defined(X64BITS)
	+ 0x50
#else
#	error Unsupported platform
#endif
	);

	g_aEntityManager.OnLevelInit(pszMapName, nullptr, this->m_sOldMap.c_str(), nullptr, false, false);
	this->m_sOldMap = std::string(pszMapName);
}

bool EntityManager::Pause(char *error, size_t maxlen)
{
	return true;
}

bool EntityManager::Unpause(char *error, size_t maxlen)
{
	return true;
}

const char *EntityManager::GetLicense()
{
	return "Public Domain";
}

const char *EntityManager::GetVersion()
{
	return "1.0.0";
}

const char *EntityManager::GetDate()
{
	return __DATE__;
}

const char *EntityManager::GetLogTag()
{
	return "ENTITY_MANAGER";
}

const char *EntityManager::GetAuthor()
{
	return "Wend4r";
}

const char *EntityManager::GetDescription()
{
	return "Plugin to manage a entities";
}

const char *EntityManager::GetName()
{
	return "Entity Manager";
}

const char *EntityManager::GetURL()
{
	return "https://github.com/mms-entity_manager";
}
