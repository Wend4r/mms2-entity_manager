/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * Metamod:Source Entity Manager
 * Written by Wend4r.
 * ======================================================

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string>
#include <functional>
#include <algorithm>

// Game SDK.
#include <eiface.h>
#include <iserver.h>
#include <entity2/entitysystem.h>
#include <gamesystems/spawngroup_manager.h>
#include <worldrenderer/iworld.h>
#include <worldrenderer/iworldrenderermgr.h>
#include <tier0/dbg.h>
#include <tier1/convar.h>

#include <dynlibutils/virtual.hpp>

#include "entity_manager.hpp"
#include "entity_manager/provider/gameresource.hpp"
#include "entity_manager/provider/spawngroup.hpp"

#define ENTITY_MANAGER_WORLD_ROOT "entity_manager"
#define ENTITY_MANAGER_PROGENITOR_WORLD_NAME "progenitor_layer"
#define ENTITY_FILTER_NAME_MAPLOAD "mapload"
#define ENTITY_FILTER_NAME_RESPAWN "cs_respawn"

const Color ENTITY_MANAGER_LOGGINING_COLOR = {125, 125, 125, 255};

class GameSessionConfiguration_t { };

SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);
SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t &, ISource2WorldSession *, const char *);
SH_DECL_HOOK2(IGameEventManager2, LoadEventsFromFile, SH_NOATTRIB, 0, int, const char *, bool);

SH_DECL_HOOK2_void(CGameEntitySystem, Spawn, SH_NOATTRIB, 0, int, const EntitySpawnInfo_t *);
SH_DECL_HOOK2_void(CGameEntitySystem, UpdateOnRemove, SH_NOATTRIB, 0, int, const EntityDeletion_t *);

SH_DECL_HOOK2_void(CSpawnGroupMgrGameSystem, AllocateSpawnGroup, SH_NOATTRIB, 0, SpawnGroupHandle_t, ISpawnGroup *);
SH_DECL_HOOK4_void(CSpawnGroupMgrGameSystem, SpawnGroupInit, SH_NOATTRIB, 0, SpawnGroupHandle_t, IEntityResourceManifest *, IEntityPrecacheConfiguration *, ISpawnGroupPrerequisiteRegistry *);
SH_DECL_HOOK4(CSpawnGroupMgrGameSystem, CreateLoadingSpawnGroup, SH_NOATTRIB, 0, ILoadingSpawnGroup *, SpawnGroupHandle_t, bool, bool, const CUtlVector<const CEntityKeyValues *> *);
SH_DECL_HOOK1_void(CSpawnGroupMgrGameSystem, SpawnGroupShutdown, SH_NOATTRIB, 0, SpawnGroupHandle_t);

static EntityManagerPlugin s_aEntityManager;
DLL_EXPORT EntityManagerPlugin *g_pEntityManager = &s_aEntityManager;  // To extern usage.

static EntityManager::Provider s_aEntityManagerProvider;
DLL_EXPORT EntityManager::Provider *g_pEntityManagerProvider = &s_aEntityManagerProvider;

static EntityManager::ProviderAgent s_aEntityManagerProviderAgent;
DLL_EXPORT EntityManager::ProviderAgent *g_pEntityManagerProviderAgent = &s_aEntityManagerProviderAgent;

DLL_EXPORT IVEngineServer *engine = NULL;
DLL_EXPORT ICvar *icvar = NULL;
DLL_EXPORT IFileSystem *filesystem = NULL;
DLL_EXPORT IServerGameDLL *server = NULL;
DLL_EXPORT IGameEventManager2 *gameeventmanager = NULL;

DLL_IMPORT CGameEntitySystem *g_pGameEntitySystem;
DLL_IMPORT CSpawnGroupMgrGameSystem *g_pSpawnGroupMgr;

// Should only be called within the active game loop (i e map should be loaded and active)
// otherwise that'll be nullptr!
CGlobalVars *GetGameGlobals()
{
	INetworkGameServer *server = g_pNetworkServerService->GetIGameServer();

	if(!server)
		return nullptr;

	return g_pNetworkServerService->GetIGameServer()->GetGlobals();
}

CGameEntitySystem *GameEntitySystem()
{
	return g_pGameEntitySystem;
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
	GET_V_IFACE_ANY(GetEngineFactory, g_pNetworkServerService, INetworkServerService, NETWORKSERVERSERVICE_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pGameResourceServiceServer, IGameResourceService, GAMERESOURCESERVICESERVER_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pWorldRendererMgr, IWorldRendererMgr, WORLD_RENDERER_MGR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetFileSystemFactory, filesystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);

	// Currently doesn't work from within mm side, use GetGameGlobals() in the mean time instead
	// gpGlobals = ismm->GetCGlobals();

	META_CONPRINTF( "Starting %s plugin.\n", this->GetName());

	SH_ADD_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &EntityManagerPlugin::OnStartupServerHook, true);

	{
		auto *pCGameEventManagerVTable = reinterpret_cast<IGameEventManager2 *>((void *)DynLibUtils::CModule(server).GetVirtualTableByName("CGameEventManager"));

		this->m_iLoadEventsFromFileId = SH_ADD_DVPHOOK(IGameEventManager2, LoadEventsFromFile, pCGameEventManagerVTable, SH_MEMBER(this, &EntityManagerPlugin::OnLoadEventsFromFileHook), false);
	}

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
			this->InitGameResource();
			this->InitGameEvents();
			this->InitSpawnGroup();

			// Load by spawn groups now.
			{
				auto pSpawnGroupMgr = (EntityManager::CSpawnGroupMgrGameSystemProvider *)g_pSpawnGroupMgr;

				auto aWarnings = this->m_aLogger.CreateWarningsScope();

				pSpawnGroupMgr->LoopBySpawnGroups([this, &aWarnings](SpawnGroupHandle_t h, CMapSpawnGroup *pMap) -> void
				{
					char sSettingsError[256];

					ISpawnGroup *pSpawnGroup = pMap->GetSpawnGroup();

					if(this->LoadSettings(pSpawnGroup, (char *)sSettingsError, sizeof(sSettingsError)))
					{
						SpawnGroupDesc_t aDesc;

						aDesc.m_hOwner = h;
						// aDesc.m_sWorldName = ENTITY_MANAGER_WORLD_ROOT CORRECT_PATH_SEPARATOR_S ENTITY_MANAGER_PROGENITOR_WORLD_NAME;
						aDesc.m_sEntityLumpName = "main lump";
						aDesc.m_sEntityFilterName = ENTITY_FILTER_NAME_MAPLOAD;
						// aDesc.m_sParentNameFixup = pSpawnGroup->GetParentNameFixup();
						aDesc.m_sLocalNameFixup = ENTITY_MANAGER_WORLD_ROOT;

						{
							char sDescriptiveName[256];

							V_snprintf(sDescriptiveName, sizeof(sDescriptiveName), "%s (Late Load JustInTime)", this->GetName());
							aDesc.m_sDescriptiveName = (const char *)sDescriptiveName;
						}

						aDesc.m_manifestLoadPriority = RESOURCE_MANIFEST_LOAD_PRIORITY_HIGH;
						aDesc.m_bCreateClientEntitiesOnLaterConnectingClients = true;
						aDesc.m_bBlockUntilLoaded = true;

						const char *pOwnerWorldName = pSpawnGroup->GetWorldName();

						IWorldReference *pOwnerWorldRef = pSpawnGroup->GetWorldReference();

						if(!s_aEntityManagerProviderAgent.CreateSpawnGroup(aDesc, pSpawnGroup->ComputeWorldOrigin(pOwnerWorldName, h, pOwnerWorldRef ? g_pWorldRendererMgr->GetGeomentryFromReference(pOwnerWorldRef) : NULL).GetOrigin()))
						{
							aWarnings.PushFormat("Failed to start creating JustInTime spawn group for \"%s\" world\n", pOwnerWorldName);
						}
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
	this->DestroyMyEntities();

	if(gameeventmanager)
	{
		this->UnhookEvents();
	}

	SH_REMOVE_HOOK_ID(this->m_iLoadEventsFromFileId);

	this->m_aSettings.Destroy();
	s_aEntityManagerProvider.Destroy();
	s_aEntityManagerProviderAgent.Destroy();

	SH_REMOVE_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &EntityManagerPlugin::OnStartupServerHook, true);

	this->DestroySpawnGroup();
	this->DestroyGameEvents();
	this->DestroyEntitySystem();
	this->DestroyGameResource();

	ConVar_Unregister();

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
	bool bResult = s_aEntityManagerProviderAgent.NotifyEntitySystemUpdated();

	if(bResult)
	{
		SH_ADD_HOOK_MEMFUNC(CGameEntitySystem, Spawn, g_pGameEntitySystem, this, &EntityManagerPlugin::OnEntitySystemSpawnHook, false);
		SH_ADD_HOOK_MEMFUNC(CGameEntitySystem, UpdateOnRemove, g_pGameEntitySystem, this, &EntityManagerPlugin::OnEntitySystemUpdateOnRemoveHook, false);
	}

	return bResult;
}

void EntityManagerPlugin::DestroyEntitySystem()
{
	SH_REMOVE_HOOK_MEMFUNC(CGameEntitySystem, Spawn, g_pGameEntitySystem, this, &EntityManagerPlugin::OnEntitySystemSpawnHook, false);
	SH_REMOVE_HOOK_MEMFUNC(CGameEntitySystem, UpdateOnRemove, g_pGameEntitySystem, this, &EntityManagerPlugin::OnEntitySystemUpdateOnRemoveHook, false);
}

bool EntityManagerPlugin::InitGameResource()
{
	bool bResult = s_aEntityManagerProviderAgent.NotifyGameResourceUpdated();

	if(bResult)
	{
		// ...
	}

	return bResult;
}

void EntityManagerPlugin::DestroyGameResource()
{
	// ...
}

bool EntityManagerPlugin::InitGameEvents()
{
	return s_aEntityManagerProviderAgent.NotifyGameEventsUpdated();
}

void EntityManagerPlugin::DestroyGameEvents()
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

bool EntityManagerPlugin::HookEvents(char *psError, size_t nMaxLength)
{
	bool bResult = !this->m_bIsHookedEvents;

	if(bResult)
	{
		// if(this->m_aRoundPreStart.Init(psError, nMaxLength))
		if(this->m_aRoundStart.Init(psError, nMaxLength))
		{
			this->m_bIsHookedEvents = true;
		}
	}
	else
	{
		strncpy(psError, "Already hooked", nMaxLength);
	}

	return bResult;
}

void EntityManagerPlugin::UnhookEvents()
{
	this->m_aRoundPreStart.Destroy();
	this->m_aRoundStart.Destroy();
	this->m_bIsHookedEvents = false;
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

bool EntityManagerPlugin::LoadSettings(ISpawnGroup *pSpawnGroup, char *psError, size_t nMaxLength)
{
	const char *pWorldName = pSpawnGroup->GetWorldName();

	if(!pWorldName || !pWorldName[0])
	{
		strncpy(psError, "World name is empty", nMaxLength);

		return false;
	}

#ifdef DEBUG
	Logger::Scope aDetails = this->m_aLogger.CreateDetailsScope();
#endif
	Logger::Scope aWarnings = this->m_aLogger.CreateWarningsScope();

	char sSpawnGroupName[MAX_SPAWN_GROUP_WORLD_NAME_LENGTH];

	{
		SpawnGroupHandle_t hOwner;

		ISpawnGroup *pOwnerSpawnGroup = pSpawnGroup;

		CMapSpawnGroup *pMap;

		CUtlVector<const char *> vecNamesByPedigree;

		while((hOwner = pOwnerSpawnGroup->GetOwnerSpawnGroup()) != INVALID_SPAWN_GROUP && (pMap = ((EntityManager::CSpawnGroupMgrGameSystemProvider *)g_pSpawnGroupMgr)->Get(hOwner)))
		{
			pOwnerSpawnGroup = pMap->GetSpawnGroup();

			vecNamesByPedigree.AddToHead(pOwnerSpawnGroup->GetWorldName());
		}

		size_t nStoredLength = 0;

		if(vecNamesByPedigree.Count())
		{
			FOR_EACH_VEC(vecNamesByPedigree, i)
			{
				V_strncpy(&sSpawnGroupName[nStoredLength], vecNamesByPedigree[i], sizeof(sSpawnGroupName) - nStoredLength);
				nStoredLength += V_strlen(vecNamesByPedigree[i]);

				if(nStoredLength + 1 < sizeof(sSpawnGroupName))
				{
					sSpawnGroupName[nStoredLength] = CORRECT_PATH_SEPARATOR;
					nStoredLength++;
				}
			}
		}

		V_strncpy(&sSpawnGroupName[nStoredLength], pWorldName, sizeof(sSpawnGroupName) - nStoredLength);
	}

	V_FixSlashes((char *)sSpawnGroupName);

	bool bResult = this->m_aSettings.Load(pSpawnGroup->GetHandle(), this->m_sBasePath.c_str(), (const char *)sSpawnGroupName, psError, nMaxLength,
#ifdef DEBUG
	                                      &aDetails,
#else
	                                      nullptr,
#endif
	                                      &aWarnings);

	if(bResult)
	{
#ifdef DEBUG
		aDetails.SendColor([this](const Color &rgba, const char *pszContent)
		{
			this->m_aLogger.Detailed(rgba, pszContent);
		});
#endif

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

	// Refresh a settings.
	{
		auto aWarnings = this->m_aLogger.CreateWarningsScope();

		auto pSpawnGroupMgr = (EntityManager::CSpawnGroupMgrGameSystemProvider *)g_pSpawnGroupMgr;

		pSpawnGroupMgr->LoopBySpawnGroups([this, &aWarnings](SpawnGroupHandle_t h, CMapSpawnGroup *pMap) -> void
		{
			ISpawnGroup *pSpawnGroup = pMap->GetSpawnGroup();

			char sSettingsError[256];

			if(!this->LoadSettings(pSpawnGroup, (char *)sSettingsError, sizeof(sSettingsError)))
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

void EntityManagerPlugin::SpawnMyEntities()
{
	auto aWarnings = this->m_aLogger.CreateWarningsScope();
	
	auto pSpawnGroupMgr = (EntityManager::CSpawnGroupMgrGameSystemProvider *)g_pSpawnGroupMgr;

	pSpawnGroupMgr->LoopBySpawnGroups([this, &aWarnings](SpawnGroupHandle_t h, CMapSpawnGroup *pMap) -> void
	{
		// char sSettingsError[256];

		ISpawnGroup *pSpawnGroup = pMap->GetSpawnGroup();

		// if(this->LoadSettings(pSpawnGroup, (char *)sSettingsError, sizeof(sSettingsError)))
		if(s_aEntityManagerProviderAgent.HasInSpawnQueue(h))
		{
			const char *pSaveFilterName = pMap->GetEntityFilterName();

			auto pSpawnGroupEx = ((EntityManager::CBaseSpawnGroupProvider *)pSpawnGroup);

			pSpawnGroupEx->SetEntityFilterName(ENTITY_FILTER_NAME_RESPAWN);

			this->m_bIsCurrentMySpawnOfEntities = true;

			{
				ILoadingSpawnGroup *pLoading = g_pSpawnGroupMgr->CreateLoadingSpawnGroup(h, true, true, NULL);

				// Spawn right now.
				pLoading->SpawnEntities();
				// pLoading->Release(); //FIXME: Maked a crash.

				pSpawnGroup->SetLoadingSpawnGroup(pLoading);
			}

			this->m_bIsCurrentMySpawnOfEntities = false;

			pSpawnGroupEx->SetEntityFilterName(pSaveFilterName);
		}
	});

	aWarnings.SendColor([this](const Color &rgba, const char *pszContent)
	{
		this->m_aLogger.Warning(rgba, pszContent);
	});
}

void EntityManagerPlugin::DestroyMyEntities()
{
	for(const auto &it : this->m_vecMyEntities)
	{
		s_aEntityManagerProviderAgent.PushDestroyQueue(it);
	}

	s_aEntityManagerProviderAgent.DestroyQueued();
}


bool EntityManagerPlugin::EraseMyEntity(CEntityInstance *pEntity)
{
	const auto &itEnd = this->m_vecMyEntities.end();

	const auto &itFound = std::find(this->m_vecMyEntities.begin(), itEnd, pEntity);

	bool bResult = itFound != itEnd;

	if(bResult)
	{
		this->m_vecMyEntities.erase(itFound);
	}

	return bResult;
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

void EntityManagerPlugin::OnStartupServerHook(const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession, const char *)
{
	s_aEntityManagerProviderAgent.Clear();

	INetworkGameServer *pNetServer = g_pNetworkServerService->GetIGameServer();

	if(pNetServer)
	{
		const char *pszMapName = pNetServer->GetMapName();

		if(g_pGameResourceServiceServer)
		{
			this->DestroyGameResource();
		}

		if(g_pGameEntitySystem)
		{
			this->DestroyEntitySystem();
		}

		if(g_pSpawnGroupMgr)
		{
			this->DestroySpawnGroup();
		}

		this->InitEntitySystem();
		this->InitGameResource();
		this->InitGameEvents();
		this->InitSpawnGroup();
	}
	else
	{
		this->m_aLogger.Warning("Failed to get a net server\n");
	}
}


int EntityManagerPlugin::OnLoadEventsFromFileHook(const char *pszFilename, bool bSearchAll)
{
	ExecuteOnce(gameeventmanager = META_IFACEPTR(IGameEventManager2));

	{
		char sError[256];

		if(!this->HookEvents((char *)sError, sizeof(sError)))
		{
			this->m_aLogger.WarningFormat("Failed to hook events: %s", sError);
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, 0);
}

void EntityManagerPlugin::OnEntitySystemSpawnHook(int iCount, const EntitySpawnInfo_t *pInfo)
{
	this->m_aLogger.MessageFormat("EntityManagerPlugin::OnEntitySystemSpawnHook(%d, %p)\n", iCount, pInfo);

#ifdef DEBUG
	auto aDetails = this->m_aLogger.CreateDetailsScope();
#endif

	{
		for(int i = 0; i < iCount; i++)
		{
			const EntitySpawnInfo_t &aInfoOne = pInfo[i];

			CEntityIdentity *pEntity = aInfoOne.m_pEntity;

#ifdef DEBUG
			int iEntity = pEntity->GetEntityIndex().Get();

			const CEntityKeyValues *pKeyValues = aInfoOne.m_pKeyValues;

			aDetails.PushFormat("-- Spawn entity (#%d) --", iEntity);

			auto aEntityDetails = Logger::Scope(LOGGER_COLOR_ENTITY_KV3, "\t");

			if(s_aEntityManagerProviderAgent.DumpEntityKeyValues(pKeyValues, EntityManager::ProviderAgent::s_eDefaultDEKVFlags, aEntityDetails, &aEntityDetails))
			{
				aDetails.PushFormat(LOGGER_COLOR_ENTITY_KV3, "%s = ", pEntity->GetClassname());
				aDetails.Push(LOGGER_COLOR_ENTITY_KV3, "{");
				aDetails += aEntityDetails;
				aDetails.Push(LOGGER_COLOR_ENTITY_KV3, "}");
			}
			else
			{
				aDetails += aEntityDetails;
			}
#endif
			if(this->m_bIsCurrentMySpawnOfEntities)
			{
				this->OnMyEntityFinish(pEntity->m_pInstance, aInfoOne.m_pKeyValues);
			}
		}
	}

#ifdef DEBUG
	aDetails.SendColor([this](const Color &rgba, const std::string sContent)
	{
		this->m_aLogger.Detailed(rgba, sContent.c_str());
	});
#endif

	SET_META_RESULT(MRES_HANDLED);
}

void EntityManagerPlugin::OnEntitySystemUpdateOnRemoveHook(int iCount, const EntityDeletion_t *pInfo)
{
	for(int i = 0; i < iCount; i++)
	{
		const auto &aInfoOne = pInfo[i];

		this->EraseMyEntity(aInfoOne.m_pEntity->m_pInstance);
	}
}

void EntityManagerPlugin::OnAllocateSpawnGroupHook(SpawnGroupHandle_t handle, ISpawnGroup *pSpawnGroup)
{
	this->m_aLogger.MessageFormat("EntityManagerPlugin::OnAllocateSpawnGroupHook(%d, %p)\n", handle, pSpawnGroup);

	// Load settings by spawn group name.
	{
		char sSettingsError[256];

		if(!this->LoadSettings(pSpawnGroup, (char *)sSettingsError, sizeof(sSettingsError)))
		{
			this->m_aLogger.WarningFormat(LOGGER_COLOR_WARNING, "Failed to load a settings: %s\n", sSettingsError);
		}
	}

	s_aEntityManagerProviderAgent.NotifyAllocateSpawnGroup(handle, pSpawnGroup);
}

ILoadingSpawnGroup *EntityManagerPlugin::OnCreateLoadingSpawnGroupHook(SpawnGroupHandle_t handle, bool bSynchronouslySpawnEntities, bool bConfirmResourcesLoaded, const CUtlVector<const CEntityKeyValues *> *pKeyValues)
{
	this->m_aLogger.MessageFormat("EntityManagerPlugin::CreateLoadingSpawnGroup(%d, bSynchronouslySpawnEntities = %s, bConfirmResourcesLoaded = %s, pKeyValues = %p)\n", handle, bSynchronouslySpawnEntities ? "true" : "false", bConfirmResourcesLoaded ? "true" : "false", pKeyValues);

	auto funcCreateLoadingSpawnGroup = &CSpawnGroupMgrGameSystem::CreateLoadingSpawnGroup;

	SET_META_RESULT(MRES_HANDLED);
	SH_GLOB_SHPTR->DoRecall();

	EntityManager::CSpawnGroupMgrGameSystemProvider *pSpawnGroupMgr = reinterpret_cast<EntityManager::CSpawnGroupMgrGameSystemProvider *>(reinterpret_cast<CSpawnGroupMgrGameSystem *>(SourceHook::RecallGetIface(SH_GLOB_SHPTR, funcCreateLoadingSpawnGroup)));

	CMapSpawnGroup *pMapSpawnGroup = pSpawnGroupMgr->Get(handle);

	CUtlVector<const CEntityKeyValues *> vecLayerEntities; // pKeyValues stored it. Control the lifecycle.

	{
		bool bIsMySpawnGroup = pMapSpawnGroup != NULL;

		if(bIsMySpawnGroup)
		{
			bIsMySpawnGroup = !V_strncmp(pMapSpawnGroup->GetLocalNameFixup(), ENTITY_MANAGER_WORLD_ROOT, sizeof(ENTITY_MANAGER_WORLD_ROOT) - 1);
		}

		{
			if(pKeyValues)
			{
				vecLayerEntities = *pKeyValues;
			}

			SpawnGroupHandle_t hTargetSpawnGroup = bIsMySpawnGroup ? pMapSpawnGroup->GetOwnerSpawnGroup() : handle;

			if(s_aEntityManagerProviderAgent.HasInSpawnQueue(hTargetSpawnGroup))
			{
				s_aEntityManagerProviderAgent.AddSpawnQueueToTail(vecLayerEntities, hTargetSpawnGroup);
				pKeyValues = &vecLayerEntities;
			}
		}
	}

	ILoadingSpawnGroup *pLoading = (pSpawnGroupMgr->*(funcCreateLoadingSpawnGroup))(handle, bSynchronouslySpawnEntities, bConfirmResourcesLoaded, pKeyValues);

	const int iCount = pLoading->EntityCount();

	if(iCount)
	{
#ifdef DEBUG
		auto aDetails = this->m_aLogger.CreateDetailsScope();
#endif

		CUtlVector<EntitySpawnInfo_t> aMyEntities;

		{
			int i = 0;

			const EntitySpawnInfo_t *pEntities = pLoading->GetEntities();

#ifdef DEBUG
			aDetails.Push("- Loading entities -");

			static const char *pszMyEntityMessages[] = 
			{
				"-- Map group entity --",
				"-- My entity --",
			};
#endif

			do
			{
				const auto &aEntity = pEntities[i];

				const CEntityKeyValues *pKeyValues = aEntity.m_pKeyValues;

				bool bInSpawnQueue = s_aEntityManagerProviderAgent.HasInSpawnQueue(pKeyValues);

#ifdef DEBUG
				aDetails.Push(pszMyEntityMessages[bInSpawnQueue]);

				auto aEntityDetails = Logger::Scope(LOGGER_COLOR_ENTITY_KV3, "\t");

				if(s_aEntityManagerProviderAgent.DumpEntityKeyValues(pKeyValues, EntityManager::ProviderAgent::s_eDefaultDEKVFlags, aEntityDetails, &aEntityDetails))
				{
					aDetails.PushFormat(LOGGER_COLOR_ENTITY_KV3, "%s = ", aEntity.m_pEntity->GetClassname());
					aDetails.Push(LOGGER_COLOR_ENTITY_KV3, "{");
					aDetails += aEntityDetails;
					aDetails.Push(LOGGER_COLOR_ENTITY_KV3, "}");
				}
				else
				{
					aDetails += aEntityDetails;
				}
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
		aDetails.SendColor([this](const Color &rgba, const char *pszContent)
		{
			this->m_aLogger.Detailed(rgba, pszContent);
		});
#endif

		if(!this->m_bIsCurrentMySpawnOfEntities)
		{
			this->ListenLoadingSpawnGroup(handle, aMyEntities.Count(), aMyEntities.Base());
		}
	}

	RETURN_META_VALUE(MRES_SUPERCEDE, pLoading);
}

void EntityManagerPlugin::OnSpawnGroupShutdownHook(SpawnGroupHandle_t handle)
{
	this->m_aLogger.MessageFormat("EntityManagerPlugin::OnSpawnGroupShutdownHook(%d)\n", handle);

	s_aEntityManagerProviderAgent.NotifyDestroySpawnGroup(handle);
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

void EntityManagerPlugin::OnMyEntityFinish(CEntityInstance *pEntity, const CEntityKeyValues *pKeyValues)
{
	this->m_aLogger.MessageFormat("EntityManagerPlugin::OnMyEntityFinish(%s (%d))\n", pEntity->GetClassname(), pEntity->GetEntityIndex().Get());

	this->m_vecMyEntities.push_back(pEntity);
}

EntityManagerPlugin::BaseEvent::BaseEvent(const char *pszName)
 :   m_pszName(pszName)
{
}

bool EntityManagerPlugin::BaseEvent::Init(char *psError, size_t nMaxLength)
{
	const char *pszMyName = this->GetName();

	bool bResult = gameeventmanager->AddListener(this, pszMyName, true);

	if(!bResult)
	{
		snprintf(psError, nMaxLength, "Failed to add \"%s\" event to listener", pszMyName);
	}

	return bResult;
}

void EntityManagerPlugin::BaseEvent::Destroy()
{
	gameeventmanager->RemoveListener(this);
}

const char *EntityManagerPlugin::BaseEvent::GetName()
{
	return this->m_pszName;
}

EntityManagerPlugin::RoundPreStartEvent::RoundPreStartEvent()
 :  BaseEvent("round_prestart")
{
}

void EntityManagerPlugin::RoundPreStartEvent::FireGameEvent(IGameEvent *pEvent)
{
	// // Won't spawns on current event.
	// s_aEntityManager.SpawnMyEntities();
}

EntityManagerPlugin::RoundStartEvent::RoundStartEvent()
 :  BaseEvent("round_start")
{
}

void EntityManagerPlugin::RoundStartEvent::FireGameEvent(IGameEvent *pEvent)
{
	s_aEntityManager.SpawnMyEntities();
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
