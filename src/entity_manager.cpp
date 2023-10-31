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
#include <gamesystems/spawngroup_manager.h>
#include <tier0/dbg.h>
#include <tier1/convar.h>

#include "entity_manager.h"
#include "entity_manager/provider/gameresource.h"
#include "entity_manager/provider/spawngroup.h"

const Color ENTITY_MANAGER_LOGGINING_COLOR = {125, 125, 125, 255};

class GameSessionConfiguration_t { };

SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);
SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t &, ISource2WorldSession *, const char *);
SH_DECL_HOOK2_void(CSpawnGroupMgrGameSystem, AllocateSpawnGroup, SH_NOATTRIB, 0, SpawnGroupHandle_t, ISpawnGroup *);
SH_DECL_HOOK4_void(CSpawnGroupMgrGameSystem, SpawnGroupInit, SH_NOATTRIB, 0, SpawnGroupHandle_t, IEntityResourceManifest *, IEntityPrecacheConfiguration *, ISpawnGroupPrerequisiteRegistry *);
SH_DECL_HOOK4(CSpawnGroupMgrGameSystem, CreateLoadingSpawnGroup, SH_NOATTRIB, 0, ILoadingSpawnGroup *, SpawnGroupHandle_t, bool, bool, const CUtlVector<const CEntityKeyValues *> *);
SH_DECL_HOOK1_void(CSpawnGroupMgrGameSystem, SpawnGroupShutdown, SH_NOATTRIB, 0, SpawnGroupHandle_t);

static EntityManagerPlugin s_aEntityManager;
EntityManagerPlugin *g_pEntityManager = &s_aEntityManager;  // To extern usage.

static EntityManager::Provider s_aEntityManagerProvider;
EntityManager::Provider *g_pEntityManagerProvider = &s_aEntityManagerProvider;

static EntityManager::ProviderAgent s_aEntityManagerProviderAgent;
EntityManager::ProviderAgent *g_pEntityManagerProviderAgent = &s_aEntityManagerProviderAgent;

IVEngineServer *engine = NULL;
ICvar *icvar = NULL;
IGameEventManager2 *gameevents = NULL;
IFileSystem *filesystem = NULL;
IServerGameDLL *server = NULL;

extern CGameEntitySystem *g_pGameEntitySystem;
extern CSpawnGroupMgrGameSystem *g_pSpawnGroupMgr;

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

EntityManagerPlugin::EntityManagerPlugin()
 :  m_aLogger(this->GetName(), [](LoggingChannelID_t nTagChannelID)
    {
    	LoggingSystem_AddTagToChannel(nTagChannelID, s_aEntityManager.GetLogTag());
    }, 0, LV_DEFAULT, ENTITY_MANAGER_LOGGINING_COLOR)
{
}

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

				auto aWarnings = this->m_aLogger.CreateWarningsScope();

				pSpawnGroupMgr->LoopBySpawnGroups([this, &aWarnings](SpawnGroupHandle_t h, CMapSpawnGroup *pMap) -> void
				{
					char sSettingsError[256];

					if(this->LoadSettings(h, pMap->GetWorldName(), (char *)sSettingsError, sizeof(sSettingsError)))
					{
						CUtlVector<const CEntityKeyValues *> vecKeyValues;

						ILoadingSpawnGroup *pMyLoading = g_pSpawnGroupMgr->CreateLoadingSpawnGroup(h, false, false, &vecKeyValues);

						// g_pSpawnGroupMgr->SpawnGroupInit(h, ((EntityManager::CEntitySystemProvider *)g_pGameEntitySystem)->GetCurrentManifest(), NULL, NULL /* Require CNetworkClientSpawnGroup_WaitForAssetLoadPrerequisit (from engine2, *(uintptr_t *)this + 11 is ISpawnGroupPrerequisiteRegistry) now (@Wend4r: needs to restore lifecycle of CSequentialPrerequisite progenitor and CNetworkClientSpawnGroup slaves )*/);
						// g_pSpawnGroupMgr->SpawnGroupSpawnEntities(h);
						// ((EntityManager::CGameResourceServiceProvider *)g_pGameResourceServiceServer)->PrecacheEntitiesAndConfirmResourcesAreLoaded(h, pMyLoading->EntityCount(), pMyLoading->GetEntities(), &pMap->GetWorldOffset()); // Precache entities now.
						// pMap->LoadEntities();
						// pMyLoading->SpawnEntities(); // Spawn created now.
						// pMyLoading->Release(); // Free CLoadingSpawnGroup.

						this->ListenLoadingSpawnGroup(h, pMyLoading->EntityCount(), pMyLoading->GetEntities());

						pMap->SetLoadingSpawnGroup(pMyLoading); // To respawn in next rounds.
					}
					else
					{
						aWarnings.PushFormat("Failed to load a settings: %s", sSettingsError);
					}
				});

				aWarnings.SendColor([this](const Color &rgba, const char *pszContent)
				{
					this->m_aLogger.Warning(rgba, pszContent);
				});
			}
		}
	}

	return true;
}

bool EntityManagerPlugin::Unload(char *error, size_t maxlen)
{
	this->DestroyMyLoadingSpawnGroupEntities();
	this->m_aSettings.Destroy();

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

bool EntityManagerPlugin::InitEntitySystem()
{
	return s_aEntityManagerProviderAgent.NotifyEntitySystemUpdated();
}

void EntityManagerPlugin::DestroyEntitySystem()
{
	// ...
}

bool EntityManagerPlugin::InitSpawnGroup()
{
	bool bResult = s_aEntityManagerProviderAgent.NotifySpawnGroupMgrUpdated();

	if(bResult)
	{
		SH_ADD_HOOK_MEMFUNC(CSpawnGroupMgrGameSystem, AllocateSpawnGroup, g_pSpawnGroupMgr, this, &EntityManagerPlugin::OnAllocateSpawnGroupHook, true);
		SH_ADD_HOOK_MEMFUNC(CSpawnGroupMgrGameSystem, CreateLoadingSpawnGroup, g_pSpawnGroupMgr, this, &EntityManagerPlugin::OnCreateLoadingSpawnGroupHook, false);
		SH_ADD_HOOK_MEMFUNC(CSpawnGroupMgrGameSystem, SpawnGroupShutdown, g_pSpawnGroupMgr, this, &EntityManagerPlugin::OnSpawnGroupShutdownHook, true);
	}

	return bResult;
}

void EntityManagerPlugin::DestroySpawnGroup()
{
	SH_REMOVE_HOOK_MEMFUNC(CSpawnGroupMgrGameSystem, AllocateSpawnGroup, g_pSpawnGroupMgr, this, &EntityManagerPlugin::OnAllocateSpawnGroupHook, true);
	SH_REMOVE_HOOK_MEMFUNC(CSpawnGroupMgrGameSystem, CreateLoadingSpawnGroup, g_pSpawnGroupMgr, this, &EntityManagerPlugin::OnCreateLoadingSpawnGroupHook, false);
	SH_REMOVE_HOOK_MEMFUNC(CSpawnGroupMgrGameSystem, SpawnGroupShutdown, g_pSpawnGroupMgr, this, &EntityManagerPlugin::OnSpawnGroupShutdownHook, true);
}

bool EntityManagerPlugin::LoadProvider(char *psError, size_t nMaxLength)
{
	char sProviderError[256];

	bool bResult = s_aEntityManagerProvider.Load(this->m_sBasePath.c_str(), (char *)sProviderError, sizeof(sProviderError));

	if(!bResult && psError)
	{
		snprintf(psError, nMaxLength, "Failed to init a provider: %s", sProviderError);
	}

	return bResult;
}

bool EntityManagerPlugin::LoadSettings(SpawnGroupHandle_t hSpawnGroup, const char *pszSpawnGroupName, char *psError, size_t nMaxLength)
{
	Logger::Scope aDetails = this->m_aLogger.CreateDetailsScope();
	Logger::Scope aWarnings = this->m_aLogger.CreateWarningsScope();

	bool bResult = this->m_aSettings.Load(hSpawnGroup, this->m_sBasePath.c_str(), pszSpawnGroupName, psError, nMaxLength, &aDetails, &aWarnings);

	if(bResult)
	{
		aDetails.SendColor([this](const Color &rgba, const char *pszContent)
		{
			this->m_aLogger.Detailed(rgba, pszContent);
		});

		aWarnings.SendColor([this](const Color &rgba, const char *pszContent)
		{
			this->m_aLogger.Warning(rgba, pszContent);
		});
	}

	return bResult;
}

void EntityManagerPlugin::OnBasePathChanged(const char *pszNewOne)
{
	this->m_sBasePath = pszNewOne;

	// Load a provider.
	{
		char sProviderError[256];

		if(!this->LoadProvider((char *)sProviderError, sizeof(sProviderError)))
		{
			this->m_aLogger.WarningFormat("Failed to load a provider: %s\n", sProviderError);
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
		this->m_aLogger.Warning(LOGGER_COLOR_WARNING, "Failed to get a net server\n");
	}
}

void EntityManagerPlugin::OnAllocateSpawnGroupHook(SpawnGroupHandle_t handle, ISpawnGroup *pSpawnGroup)
{
	this->m_aLogger.MessageFormat("EntityManagerPlugin::OnAllocateSpawnGroupHook(%d, %s)\n", handle, pSpawnGroup->GetWorldName());

	// Load settings by spawn group name
	{
		char sSettingsError[256];

		if(!this->LoadSettings(handle, pSpawnGroup->GetWorldName(), (char *)sSettingsError, sizeof(sSettingsError)))
		{
			this->m_aLogger.WarningFormat(LOGGER_COLOR_WARNING, "Failed to load a settings: %s\n", sSettingsError);
		}
	}
}

ILoadingSpawnGroup *EntityManagerPlugin::OnCreateLoadingSpawnGroupHook(SpawnGroupHandle_t handle, bool bSynchronouslySpawnEntities, bool bConfirmResourcesLoaded, const CUtlVector<const CEntityKeyValues *> *pKeyValues)
{
	this->m_aLogger.MessageFormat("EntityManagerPlugin::CreateLoadingSpawnGroup(%d, bSynchronouslySpawnEntities = %s, bConfirmResourcesLoaded = %s, pKeyValues = %p)\n", handle, bSynchronouslySpawnEntities ? "true" : "false", bConfirmResourcesLoaded ? "true" : "false", pKeyValues);

	auto funcCreateLoadingSpawnGroup = &CSpawnGroupMgrGameSystem::CreateLoadingSpawnGroup;

	SET_META_RESULT(MRES_HANDLED);
	SH_GLOB_SHPTR->DoRecall();

	s_aEntityManagerProviderAgent.AddSpawnQueueToTail(const_cast<CUtlVector<const CEntityKeyValues *> *>(pKeyValues), handle);

	ILoadingSpawnGroup *pLoading = (SourceHook::RecallGetIface(SH_GLOB_SHPTR, funcCreateLoadingSpawnGroup)->*(funcCreateLoadingSpawnGroup))(handle, bSynchronouslySpawnEntities, bConfirmResourcesLoaded, pKeyValues);

	const int iCount = pLoading->EntityCount();

	if(iCount)
	{
#ifdef DEBUG
		auto aMessages = this->m_aLogger.CreateDetailsScope();
#endif

		CUtlVector<EntitySpawnInfo_t> aMyEntities;

		{
			int i = 0;

			const EntitySpawnInfo_t *pEntities = pLoading->GetEntities();

#ifdef DEBUG
			static const char *pszMyEntityMessages[] = 
			{
				"Map group entity #%d:",
				"My entity #%d:",
			};

			aMessages.Push("-----");
#endif

			do
			{
				const auto &aEntity = pEntities[i];

				bool bInSpawnQueue = s_aEntityManagerProviderAgent.HasInSpawnQueue(aEntity.m_pKeyValues);

#ifdef DEBUG
				aMessages.PushFormat(pszMyEntityMessages[bInSpawnQueue], aEntity.m_pEntity->GetEntityIndex().Get());
				s_aEntityManagerProviderAgent.DumpEntityKeyValues(aEntity.m_pKeyValues, aMessages);
				aMessages.Push("-----");
#endif

				if(bInSpawnQueue)
				{
					aMyEntities.AddToTail(aEntity);
				}

				i++;
			}
			while(i < iCount);
		}

#ifdef DEBUG
		aMessages.SendColor([this](const Color &rgba, const char *pszContent)
		{
			this->m_aLogger.Detailed(rgba, pszContent);
		});
#endif

		this->ListenLoadingSpawnGroup(handle, aMyEntities.Count(), aMyEntities.Base());
	}

	s_aEntityManagerProviderAgent.ReleaseSpawnQueued(handle);

	RETURN_META_VALUE(MRES_SUPERCEDE, pLoading);
}

void EntityManagerPlugin::OnSpawnGroupShutdownHook(SpawnGroupHandle_t handle)
{
	this->m_aLogger.MessageFormat("EntityManagerPlugin::OnSpawnGroupShutdownHook(%d)\n", handle);
}

void EntityManagerPlugin::ListenLoadingSpawnGroup(SpawnGroupHandle_t hSpawnGroup, const int iCount, const EntitySpawnInfo_t *pEntities, CEntityInstance *pListener)
{
	this->m_aLogger.MessageFormat("EntityManagerPlugin::ListenLoadingSpawnGroup(%d, %d, %p, %p)\n", hSpawnGroup, iCount, pEntities, pListener);

	if(iCount)
	{
		int i = 0;

		do
		{
			const auto &aEntity = pEntities[i];

			((EntityManager::CEntitySystemProvider *)g_pGameEntitySystem)->ListenForEntityInSpawnGroupToFinish(hSpawnGroup, aEntity.m_pEntity->m_pInstance, aEntity.m_pKeyValues, 0, UtlMakeDelegate(this, &EntityManagerPlugin::OnMyEntityFinish));
			i++;

		}
		while(i < iCount);
	}
}

int EntityManagerPlugin::DestroyMyLoadingSpawnGroupEntities()
{
	auto &aIdenties = this->m_vecMyEntities;

	const int iCount = aIdenties.Count();

	if(iCount)
	{
		int j = 0;

		do
		{
			s_aEntityManagerProviderAgent.PushDestroyQueue(aIdenties[j]);
			j++;
		}
		while(j < iCount);

		s_aEntityManagerProviderAgent.DestroyQueued();
	}

	aIdenties.Purge();

	return iCount;
}

void EntityManagerPlugin::OnMyEntityFinish(CEntityInstance *pEntity, const CEntityKeyValues *pKeyValues)
{
	this->m_aLogger.MessageFormat("EntityManagerPlugin::OnMyEntityFinish(%s (%d))\n", pEntity->GetClassname(), pEntity->GetEntityIndex().Get());

	this->m_vecMyEntities.AddToTail(pEntity);
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
