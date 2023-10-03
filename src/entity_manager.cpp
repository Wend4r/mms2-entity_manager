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
#include <tier0/dbg.h>
#include <tier1/convar.h>
#include <eiface.h>
#include <iserver.h>

#include "entity_manager.h"

SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);

EntityManager g_aEntityManager;

ICvar *icvar = NULL;
IVEngineServer *engine = NULL;
IFileSystem *filesystem = NULL;
IServerGameDLL *server = NULL;

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

	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetFileSystemFactory, filesystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);

	// Currently doesn't work from within mm side, use GetGameGlobals() in the mean time instead
	// gpGlobals = ismm->GetCGlobals();

	META_CONPRINTF( "Starting %s plugin.\n", this->GetName());

	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &EntityManager::OnGameFrameHook, true);

	META_CONPRINTF( "All hooks started!\n" );

	g_pCVar = icvar;
	ConVar_Register(FCVAR_RELEASE | FCVAR_GAMEDLL);

	return true;
}

bool EntityManager::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &EntityManager::OnGameFrameHook, true);

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
	META_CONPRINTF("OnLevelInit(%s)\n", pszMapName);

	{
		char sBuffer[256];

		if(!this->m_aSettings.Load(this->m_sBasePath.c_str(), pszMapName, (char *)sBuffer, sizeof(sBuffer)))
		{
			g_SMAPI->LogMsg(g_PLAPI, "Failed to load settings: %s\n", sBuffer);
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
