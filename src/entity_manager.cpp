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
#include "entity_manager.h"
#include "iserver.h"

SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);
SH_DECL_HOOK4_void(IServerGameClients, ClientActive, SH_NOATTRIB, 0, CPlayerSlot, bool, const char *, uint64);
SH_DECL_HOOK5_void(IServerGameClients, ClientDisconnect, SH_NOATTRIB, 0, CPlayerSlot, int, const char *, uint64, const char *);
SH_DECL_HOOK4_void(IServerGameClients, ClientPutInServer, SH_NOATTRIB, 0, CPlayerSlot, char const *, int, uint64);
SH_DECL_HOOK1_void(IServerGameClients, ClientSettingsChanged, SH_NOATTRIB, 0, CPlayerSlot );
SH_DECL_HOOK6_void(IServerGameClients, OnClientConnected, SH_NOATTRIB, 0, CPlayerSlot, const char*, uint64, const char *, const char *, bool);
SH_DECL_HOOK6(IServerGameClients, ClientConnect, SH_NOATTRIB, 0, bool, CPlayerSlot, const char*, uint64, const char *, bool, CBufferString *);
SH_DECL_HOOK2(IGameEventManager2, FireEvent, SH_NOATTRIB, 0, bool, IGameEvent *, bool);

SH_DECL_HOOK2_void( IServerGameClients, ClientCommand, SH_NOATTRIB, 0, CPlayerSlot, const CCommand & );

EntityManager g_aEntityManager;
IServerGameDLL *server = NULL;
IServerGameClients *gameclients = NULL;
IVEngineServer *engine = NULL;
IGameEventManager2 *gameevents = NULL;
ICvar *icvar = NULL;

// Should only be called within the active game loop (i e map should be loaded and active)
// otherwise that'll be nullptr!
CGlobalVars *GetGameGlobals()
{
	INetworkGameServer *server = g_pNetworkServerService->GetIGameServer();

	if(!server)
		return nullptr;

	return g_pNetworkServerService->GetIGameServer()->GetGlobals();
}

#if 0
// Currently unavailable, requires hl2sdk work!
ConVar sample_cvar("em_cvar", "42", 0);
#endif

CON_COMMAND_F(em_command, "Entity Manager command", FCVAR_NONE)
{
	META_CONPRINTF( "Command called by %d. Command: %s\n", context.GetPlayerSlot(), args.GetCommandString() );
}

PLUGIN_EXPOSE(EntityManager, g_aEntityManager);
bool EntityManager::Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	GET_V_IFACE_CURRENT(GetEngineFactory, engine, IVEngineServer, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);
	GET_V_IFACE_ANY(GetServerFactory, gameclients, IServerGameClients, INTERFACEVERSION_SERVERGAMECLIENTS);
	GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);

	// Currently doesn't work from within mm side, use GetGameGlobals() in the mean time instead
	// gpGlobals = ismm->GetCGlobals();

	META_CONPRINTF( "Starting %s plugin.\n", this->GetName());

	SH_ADD_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &EntityManager::Hook_GameFrame, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientActive, gameclients, this, &EntityManager::Hook_ClientActive, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, gameclients, this, &EntityManager::Hook_ClientDisconnect, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, gameclients, this, &EntityManager::Hook_ClientPutInServer, true);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientSettingsChanged, gameclients, this, &EntityManager::Hook_ClientSettingsChanged, false);
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, OnClientConnected, gameclients, this, &EntityManager::Hook_OnClientConnected, false);
	SH_ADD_HOOK_MEMFUNC( IServerGameClients, ClientConnect, gameclients, this, &EntityManager::Hook_ClientConnect, false );
	SH_ADD_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &EntityManager::Hook_ClientCommand, false);

	META_CONPRINTF( "All hooks started!\n" );

	g_pCVar = icvar;
	ConVar_Register( FCVAR_RELEASE | FCVAR_CLIENT_CAN_EXECUTE | FCVAR_GAMEDLL );

	return true;
}

bool EntityManager::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK_MEMFUNC(IServerGameDLL, GameFrame, server, this, &EntityManager::Hook_GameFrame, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientActive, gameclients, this, &EntityManager::Hook_ClientActive, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientDisconnect, gameclients, this, &EntityManager::Hook_ClientDisconnect, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientPutInServer, gameclients, this, &EntityManager::Hook_ClientPutInServer, true);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientSettingsChanged, gameclients, this, &EntityManager::Hook_ClientSettingsChanged, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, OnClientConnected, gameclients, this, &EntityManager::Hook_OnClientConnected, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientConnect, gameclients, this, &EntityManager::Hook_ClientConnect, false);
	SH_REMOVE_HOOK_MEMFUNC(IServerGameClients, ClientCommand, gameclients, this, &EntityManager::Hook_ClientCommand, false);

	return true;
}

void EntityManager::AllPluginsLoaded()
{
	/* This is where we'd do stuff that relies on the mod or other plugins 
	 * being initialized (for example, cvars added and events registered).
	 */
}

void EntityManager::Hook_ClientActive( CPlayerSlot slot, bool bLoadGame, const char *pszName, uint64 xuid )
{
	META_CONPRINTF( "Hook_ClientActive(%d, %d, \"%s\", %d)\n", slot, bLoadGame, pszName, xuid );
}

void EntityManager::Hook_ClientCommand( CPlayerSlot slot, const CCommand &args )
{
	META_CONPRINTF( "Hook_ClientCommand(%d, \"%s\")\n", slot, args.GetCommandString() );
}

void EntityManager::Hook_ClientSettingsChanged( CPlayerSlot slot )
{
	META_CONPRINTF( "Hook_ClientSettingsChanged(%d)\n", slot );
}

void EntityManager::Hook_OnClientConnected( CPlayerSlot slot, const char *pszName, uint64 xuid, const char *pszNetworkID, const char *pszAddress, bool bFakePlayer )
{
	META_CONPRINTF( "Hook_OnClientConnected(%d, \"%s\", %d, \"%s\", \"%s\", %d)\n", slot, pszName, xuid, pszNetworkID, pszAddress, bFakePlayer );
}

bool EntityManager::Hook_ClientConnect( CPlayerSlot slot, const char *pszName, uint64 xuid, const char *pszNetworkID, bool unk1, CBufferString *pRejectReason )
{
	META_CONPRINTF( "Hook_ClientConnect(%d, \"%s\", %d, \"%s\", %d, \"%s\")\n", slot, pszName, xuid, pszNetworkID, unk1, pRejectReason->ToGrowable()->Get() );

	RETURN_META_VALUE(MRES_IGNORED, true);
}

void EntityManager::Hook_ClientPutInServer( CPlayerSlot slot, char const *pszName, int type, uint64 xuid )
{
	META_CONPRINTF( "Hook_ClientPutInServer(%d, \"%s\", %d, %d, %d)\n", slot, pszName, type, xuid );
}

void EntityManager::Hook_ClientDisconnect( CPlayerSlot slot, int reason, const char *pszName, uint64 xuid, const char *pszNetworkID )
{
	META_CONPRINTF( "Hook_ClientDisconnect(%d, %d, \"%s\", %d, \"%s\")\n", slot, reason, pszName, xuid, pszNetworkID );
}

void EntityManager::Hook_GameFrame( bool simulating, bool bFirstTick, bool bLastTick )
{
	/**
	 * simulating:
	 * ***********
	 * true  | game is ticking
	 * false | game is not ticking
	 */
}

// Potentially might not work
void EntityManager::OnLevelInit( char const *pMapName,
									 char const *pMapEntities,
									 char const *pOldLevel,
									 char const *pLandmarkName,
									 bool loadGame,
									 bool background )
{
	META_CONPRINTF("OnLevelInit(%s)\n", pMapName);
}

// Potentially might not work
void EntityManager::OnLevelShutdown()
{
	META_CONPRINTF("OnLevelShutdown()\n");
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
