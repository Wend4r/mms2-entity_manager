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

#include <entity_manager.hpp>
#include <entity_manager/provider/gameresource.hpp>
#include <entity_manager/provider/spawngroup.hpp>

#include <stdio.h>
#include <string>
#include <functional>
#include <algorithm>

// Game SDK.
#include <eiface.h>
#include <igamesystemfactory.h>
#include <iserver.h>
#include <entity2/entitysystem.h>
#include <gamesystems/spawngroup_manager.h>
#include <worldrenderer/iworld.h>
#include <worldrenderer/iworldrenderermgr.h>
#include <tier0/dbg.h>
#include <tier1/convar.h>

#include <dynlibutils/virtual.hpp>

#define ENTITY_MANAGER_WORLD_ROOT "entity_manager"
#define ENTITY_MANAGER_PROGENITOR_WORLD_NAME "progenitor_layer"
#define ENTITY_FILTER_NAME_MAPLOAD "mapload"
#define ENTITY_FILTER_NAME_RESPAWN "cs_respawn"

const Color ENTITY_MANAGER_LOGGINING_COLOR = {125, 125, 125, 255};

SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);
SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t &, ISource2WorldSession *, const char *);
SH_DECL_HOOK2(IGameEventManager2, LoadEventsFromFile, SH_NOATTRIB, 0, int, const char *, bool);

SH_DECL_HOOK2_void(CGameEntitySystem, Spawn, SH_NOATTRIB, 0, int, const EntitySpawnInfo_t *);
SH_DECL_HOOK2_void(CGameEntitySystem, UpdateOnRemove, SH_NOATTRIB, 0, int, const EntityDeletion_t *);

SH_DECL_HOOK1_void(CBaseGameSystemFactory, SetGlobalPtr, SH_NOATTRIB, 0, void *);

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

static EntityManager::CSpawnGroupAccess s_aEntityManagerSpawnGroup;
EntityManager::CSpawnGroupAccess *g_pEntityManagerSpawnGroup = &s_aEntityManagerSpawnGroup;

IVEngineServer *engine = NULL;
ICvar *icvar = NULL;
IFileSystem *filesystem = NULL;
IServerGameDLL *server = NULL;
IGameEventManager2 *gameeventmanager = NULL;

extern CGameEntitySystem *g_pGameEntitySystem;

extern CBaseGameSystemFactory *g_pGSFactoryCSpawnGroupMgrGameSystem;
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

CGameEntitySystem *GameEntitySystem()
{
	return g_pGameEntitySystem;
}

CEntityInstance* CEntityHandle::Get() const
{
	return GameEntitySystem()->GetEntityInstance( *this );
}

PLUGIN_EXPOSE(EntityManagerPlugin, s_aEntityManager);

EntityManagerPlugin::EntityManagerPlugin()
 :  m_aLogger(GetName(), [](LoggingChannelID_t nTagChannelID)
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

	META_CONPRINTF( "Starting %s plugin.\n", GetName());

	SH_ADD_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &EntityManagerPlugin::OnStartupServerHook, true);

	META_CONPRINTF( "All hooks started!\n" );

	g_pCVar = icvar;
	ConVar_Register(FCVAR_RELEASE | FCVAR_GAMEDLL);

	// Initialize and load a provider.
	{
		GameData::CBufferStringVector vecMessages;

		bool bResult = s_aEntityManagerProvider.Init(vecMessages);

		auto aWarnings = m_aLogger.CreateWarningsScope();

		FOR_EACH_VEC(vecMessages, i)
		{
			auto &aMessage = vecMessages[i];

			aWarnings.Push(aMessage.Get());
		}

		aWarnings.SendColor([this](Color rgba, const CUtlString &sContext)
		{
			m_aLogger.Warning(rgba, sContext);
		});

		if(bResult)
		{
			if(!LoadProvider())
			{
				snprintf(error, maxlen, "Failed to load a provider");

				return false;
			}
		}
		else
		{
			snprintf(error, maxlen, "Failed to init a provider");

			return false;
		}
	}

	// Initialize a provider agent.
	{
		if(!s_aEntityManagerProviderAgent.Init())
		{
			snprintf(error, maxlen, "Failed to init a provider agent");

			return false;
		}
	}

	// Initialize a settings.
	{
		char sSettingsError[256];

		if(!m_aSettings.Init(sSettingsError, sizeof(sSettingsError)))
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
			InitEntitySystem();
			InitGameResource();
			InitGameSystem();
			InitGameEvents();

			// Load by spawn groups now.
			{
				auto *pSpawnGroupManagerProvider = reinterpret_cast<IEntityManager::CSpawnGroupProvider *>(g_pEntityManagerSpawnGroup);

				assert(pSpawnGroupManagerProvider->GetManager());

				auto aWarnings = m_aLogger.CreateWarningsScope();

				pSpawnGroupManagerProvider->LoopBySpawnGroups([this, &aWarnings](SpawnGroupHandle_t h, CMapSpawnGroup *pMap) -> void
				{
					char sSettingsError[256];

					ISpawnGroup *pSpawnGroup = pMap->GetSpawnGroup();

					if(LoadSettings(pSpawnGroup, (char *)sSettingsError, sizeof(sSettingsError)))
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

							V_snprintf(sDescriptiveName, sizeof(sDescriptiveName), "%s (Late Load JustInTime)", GetName());
							aDesc.m_sDescriptiveName = (const char *)sDescriptiveName;
						}

						aDesc.m_manifestLoadPriority = RESOURCE_MANIFEST_LOAD_PRIORITY_HIGH;
						aDesc.m_bCreateClientEntitiesOnLaterConnectingClients = true;
						aDesc.m_bBlockUntilLoaded = true;

						const char *pOwnerWorldName = pSpawnGroup->GetWorldName();

						IWorldReference *pOwnerWorldRef = pSpawnGroup->GetWorldReference();

						Vector aWorldOrigin =
#if 0
							pSpawnGroup->ComputeWorldOrigin(pOwnerWorldName, h, pOwnerWorldRef ? g_pWorldRendererMgr->GetGeomentryFromReference(pOwnerWorldRef) : NULL).GetOrigin();
#else
							{0.0f, 0.0f, 0.0f};
#endif

						auto *pSpawnGroupInstance = s_aEntityManagerProviderAgent.CreateSpawnGroup();

						if(!pSpawnGroupInstance->Load(aDesc, aWorldOrigin))
						{
							aWarnings.PushFormat("Failed to start creating JustInTime spawn group for \"%s\" world\n", pOwnerWorldName);
						}
					}
					else
					{
						aWarnings.PushFormat("Failed to load a settings: %s", sSettingsError);
					}
				});

				aWarnings.SendColor([this](Color rgba, const CUtlString &sContext)
				{
					m_aLogger.Warning(rgba, sContext);
				});
			}
		}
	}

	g_SMAPI->AddListener(static_cast<ISmmPlugin *>(this), static_cast<IMetamodListener *>(this));

	return true;
}

bool EntityManagerPlugin::Unload(char *error, size_t maxlen)
{
	DestroyMyEntities();

	if(gameeventmanager)
	{
		UnhookEvents();
	}

	m_aSettings.Destroy();
	s_aEntityManagerProvider.Destroy();
	s_aEntityManagerProviderAgent.Destroy();

	SH_REMOVE_HOOK_MEMFUNC(INetworkServerService, StartupServer, g_pNetworkServerService, this, &EntityManagerPlugin::OnStartupServerHook, true);

	DestroySpawnGroup();
	DestroyGameEvents();
	DestroyEntitySystem();
	DestroyGameSystem();
	DestroyGameResource();

	ConVar_Unregister();

	return true;
}

void EntityManagerPlugin::AllPluginsLoaded()
{
	/* This is where we'd do stuff that relies on the mod or other plugins 
	 * being initialized (for example, cvars added and events registered).
	 */
}

const char *EntityManagerPlugin::GetAuthor()      { return META_PLUGIN_AUTHOR; }
const char *EntityManagerPlugin::GetName()        { return META_PLUGIN_NAME; }
const char *EntityManagerPlugin::GetDescription() { return META_PLUGIN_DESCRIPTION; }
const char *EntityManagerPlugin::GetURL()         { return META_PLUGIN_URL; }
const char *EntityManagerPlugin::GetLicense()     { return META_PLUGIN_LICENSE; }
const char *EntityManagerPlugin::GetVersion()     { return META_PLUGIN_VERSION; }
const char *EntityManagerPlugin::GetDate()        { return META_PLUGIN_DATE; }
const char *EntityManagerPlugin::GetLogTag()      { return META_PLUGIN_LOG_TAG; }

void *EntityManagerPlugin::OnMetamodQuery(const char *iface, int *ret)
{
	if(!strcmp(iface, ENTITY_MANAGER_INTERFACE_NAME))
	{
		if(ret)
		{
			*ret = META_IFACE_OK;
		}

		return static_cast<IEntityManager *>(this);
	}

	if(ret)
	{
		*ret = META_IFACE_FAILED;
	}

	return nullptr;
}

EntityManagerPlugin::IProviderAgent *EntityManagerPlugin::GetProviderAgent()
{
	return dynamic_cast<IProviderAgent *>(g_pEntityManagerProviderAgent);
}

EntityManagerPlugin::CSpawnGroupProvider *EntityManagerPlugin::GetSpawnGroupManager()
{
	return reinterpret_cast<CSpawnGroupProvider *>(g_pEntityManagerSpawnGroup);
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


bool EntityManagerPlugin::InitGameSystem()
{
	bool bResult = s_aEntityManagerProviderAgent.NotifyGameSystemUpdated();

	if(bResult)
	{
		if(g_pGSFactoryCSpawnGroupMgrGameSystem)
		{
			s_aEntityManagerProviderAgent.NotifySpawnGroupMgrUpdated();

			if(g_pSpawnGroupMgr)
			{
				g_pGSFactoryCSpawnGroupMgrGameSystem->SetGlobalPtr(g_pSpawnGroupMgr);
				InitSpawnGroup(g_pSpawnGroupMgr);
			}

			SH_ADD_HOOK_MEMFUNC(CBaseGameSystemFactory, SetGlobalPtr, g_pGSFactoryCSpawnGroupMgrGameSystem, this, &EntityManagerPlugin::OnGSFactoryCSpawnGroupMgrGameSystemSetGlobalStrHook, false);
		}
	}

	return bResult;
}

void EntityManagerPlugin::DestroyGameSystem()
{
	if(g_pGSFactoryCSpawnGroupMgrGameSystem)
	{
		SH_REMOVE_HOOK_MEMFUNC(CBaseGameSystemFactory, SetGlobalPtr, g_pGSFactoryCSpawnGroupMgrGameSystem, this, &EntityManagerPlugin::OnGSFactoryCSpawnGroupMgrGameSystemSetGlobalStrHook, false);
	}
}

bool EntityManagerPlugin::InitGameEvents()
{
	bool bResult = s_aEntityManagerProviderAgent.NotifyGameEventsUpdated();

	if(bResult)
	{
		char sError[256];

		if(!HookEvents(sError, sizeof(sError)))
		{
			m_aLogger.WarningFormat("Failed to hook events: %s\n", sError);
		}
	}

	return bResult;
}

void EntityManagerPlugin::DestroyGameEvents()
{
	UnhookEvents();
}

bool EntityManagerPlugin::InitSpawnGroup(CSpawnGroupMgrGameSystem *pSpawnGroupManager)
{
	bool bResult = s_aEntityManagerProviderAgent.NotifySpawnGroupMgrUpdated(pSpawnGroupManager);

	if(bResult)
	{
		SH_ADD_HOOK_MEMFUNC(CSpawnGroupMgrGameSystem, AllocateSpawnGroup, pSpawnGroupManager, this, &EntityManagerPlugin::OnAllocateSpawnGroupHook, true);
		SH_ADD_HOOK_MEMFUNC(CSpawnGroupMgrGameSystem, SpawnGroupInit, pSpawnGroupManager, this, &EntityManagerPlugin::OnSpawnGroupInitHook, true);
		SH_ADD_HOOK_MEMFUNC(CSpawnGroupMgrGameSystem, CreateLoadingSpawnGroup, pSpawnGroupManager, this, &EntityManagerPlugin::OnCreateLoadingSpawnGroupHook, false);
		SH_ADD_HOOK_MEMFUNC(CSpawnGroupMgrGameSystem, SpawnGroupShutdown, pSpawnGroupManager, this, &EntityManagerPlugin::OnSpawnGroupShutdownHook, true);
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
	bool bResult = !m_bIsHookedEvents;

	if(bResult)
	{
		// if(m_aRoundPreStart.Init(psError, nMaxLength))
		if(m_aRoundStart.Init(psError, nMaxLength))
		{
			m_bIsHookedEvents = true;
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
	m_aRoundStart.Destroy();
	m_aRoundPreStart.Destroy();

	m_bIsHookedEvents = false;
}

bool EntityManagerPlugin::LoadProvider()
{
	GameData::CBufferStringVector vecMessages;

	bool bResult = s_aEntityManagerProvider.Load(m_sBasePath.c_str(), vecMessages);

	// Print messages
	{
		Logger::Scope aWarnings = m_aLogger.CreateWarningsScope();

		FOR_EACH_VEC(vecMessages, i)
		{
			auto &aMessage = vecMessages[i];

			aWarnings.Push(aMessage.Get());

			aWarnings.SendColor([this](Color rgba, const CUtlString &sContext)
			{
				m_aLogger.Warning(rgba, sContext);
			});
		}
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

#ifdef _DEBUG
	Logger::Scope aDetails = m_aLogger.CreateDetailsScope();
#endif
	Logger::Scope aWarnings = m_aLogger.CreateWarningsScope();

	char sSpawnGroupName[MAX_SPAWN_GROUP_WORLD_NAME_LENGTH];

	{
		SpawnGroupHandle_t hOwner;

		ISpawnGroup *pOwnerSpawnGroup = pSpawnGroup;

		CMapSpawnGroup *pMap;

		CUtlVector<const char *> vecNamesByPedigree;

		while((hOwner = pOwnerSpawnGroup->GetOwnerSpawnGroup()) != INVALID_SPAWN_GROUP && (pMap = s_aEntityManagerSpawnGroup.Get(hOwner)))
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

	bool bResult = m_aSettings.Load(pSpawnGroup->GetHandle(), m_sBasePath.c_str(), (const char *)sSpawnGroupName, psError, nMaxLength,
#ifdef _DEBUG
	                                      &aDetails,
#else
	                                      nullptr,
#endif
	                                      &aWarnings);

	if(bResult)
	{
#ifdef _DEBUG
		aDetails.SendColor([this](Color rgba, const CUtlString &sContext)
		{
			m_aLogger.Detailed(rgba, sContext);
		});
#endif

		aWarnings.SendColor([this](Color rgba, const CUtlString &sContext)
		{
			m_aLogger.Warning(rgba, sContext);
		});
	}

	return bResult;
}

void EntityManagerPlugin::OnBasePathChanged(const char *pszNewOne)
{
	m_sBasePath = pszNewOne;

	// Load a provider.
	{
		char sProviderError[256];

		if(!LoadProvider())
		{
			m_aLogger.Warning("Failed to load a provider\n");
		}
	}

	// Refresh a settings.
	{
		auto aWarnings = m_aLogger.CreateWarningsScope();

		auto *pSpawnGroupManagerProvider = reinterpret_cast<IEntityManager::CSpawnGroupProvider *>(g_pEntityManagerSpawnGroup);

		Assert(pSpawnGroupManagerProvider->GetManager());

		pSpawnGroupManagerProvider->LoopBySpawnGroups([this, &aWarnings](SpawnGroupHandle_t h, CMapSpawnGroup *pMap) -> void
		{
			ISpawnGroup *pSpawnGroup = pMap->GetSpawnGroup();

			char sSettingsError[256];

			if(!LoadSettings(pSpawnGroup, (char *)sSettingsError, sizeof(sSettingsError)))
			{
				aWarnings.PushFormat("Failed to load a settings: %s", sSettingsError);
			}
		});

		aWarnings.SendColor([this](Color rgba, const CUtlString &sContext)
		{
			m_aLogger.Warning(rgba, sContext);
		});
	}
}

void EntityManagerPlugin::SpawnMyEntities()
{
	auto aWarnings = m_aLogger.CreateWarningsScope();

	auto *pSpawnGroupManagerProvider = reinterpret_cast<IEntityManager::CSpawnGroupProvider *>(g_pEntityManagerSpawnGroup);

	auto *pSpawnGroupManager = pSpawnGroupManagerProvider->GetManager();

	Assert(pSpawnGroupManager);

	pSpawnGroupManagerProvider->LoopBySpawnGroups([this, &aWarnings, &pSpawnGroupManager](SpawnGroupHandle_t h, CMapSpawnGroup *pMap) -> void
	{
		// char sSettingsError[256];

		ISpawnGroup *pSpawnGroup = pMap->GetSpawnGroup();

		// if(LoadSettings(pSpawnGroup, (char *)sSettingsError, sizeof(sSettingsError)))
		if(s_aEntityManagerProviderAgent.HasInSpawnQueue(h))
		{
			const char *pSaveFilterName = pMap->GetEntityFilterName();

			auto *pSpawnGroupEx = ((EntityManager::CBaseSpawnGroupProvider *)pSpawnGroup);

			pSpawnGroupEx->SetEntityFilterName(ENTITY_FILTER_NAME_RESPAWN);

			m_bIsCurrentMySpawnOfEntities = true;

			{
				ILoadingSpawnGroup *pLoading = pSpawnGroupManager->CreateLoadingSpawnGroup(h, true, true, NULL);

				// Spawn right now.
				pLoading->SpawnEntities();
				// pLoading->Release(); //FIXME: Maked a crash.

				pSpawnGroup->SetLoadingSpawnGroup(pLoading);
			}

			m_bIsCurrentMySpawnOfEntities = false;

			pSpawnGroupEx->SetEntityFilterName(pSaveFilterName);
		}
	});

	aWarnings.SendColor([this](Color rgba, const CUtlString &sContext)
	{
		m_aLogger.Warning(rgba, sContext);
	});
}

void EntityManagerPlugin::DestroyMyEntities()
{
	for(const auto &it : m_vecMyEntities)
	{
		s_aEntityManagerProviderAgent.PushDestroyQueue(it);
	}

	s_aEntityManagerProviderAgent.ExecuteDestroyQueued();
}


bool EntityManagerPlugin::EraseMyEntity(CEntityInstance *pEntity)
{
	const auto &itEnd = m_vecMyEntities.end();

	const auto &itFound = std::find(m_vecMyEntities.begin(), itEnd, pEntity);

	bool bResult = itFound != itEnd;

	if(bResult)
	{
		m_vecMyEntities.erase(itFound);
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

	OnBasePathChanged(args[1]);
	Msg("Base path is \"%s\"\n", m_sBasePath.c_str());
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
			DestroyGameResource();
		}

		if(g_pGameEntitySystem)
		{
			DestroyEntitySystem();
		}

		if(g_pSpawnGroupMgr)
		{
			DestroySpawnGroup();
		}

		InitEntitySystem();
		InitGameResource();
		InitGameSystem();
		InitGameEvents();
	}
	else
	{
		m_aLogger.Warning("Failed to get a net server\n");
	}
}

void EntityManagerPlugin::OnEntitySystemSpawnHook(int iCount, const EntitySpawnInfo_t *pInfo)
{
	if(m_aLogger.IsChannelEnabled(LV_DETAILED))
	{
		m_aLogger.DetailedFormat("EntityManagerPlugin::OnEntitySystemSpawnHook(%d, %p)\n", iCount, pInfo);
	}

#ifdef _DEBUG
	auto aDetails = m_aLogger.CreateDetailsScope();
#endif

	{
		for(int i = 0; i < iCount; i++)
		{
			const EntitySpawnInfo_t &aInfoOne = pInfo[i];

			CEntityIdentity *pEntity = aInfoOne.m_pEntity;

#ifdef _DEBUG
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
			if(m_bIsCurrentMySpawnOfEntities)
			{
				OnMyEntityFinish(pEntity->m_pInstance, aInfoOne.m_pKeyValues);
			}
		}
	}

#ifdef _DEBUG
	aDetails.SendColor([this](Color rgba, const CUtlString &sContent)
	{
		m_aLogger.Detailed(rgba, sContent);
	});
#endif

	SET_META_RESULT(MRES_HANDLED);
}

void EntityManagerPlugin::OnEntitySystemUpdateOnRemoveHook(int iCount, const EntityDeletion_t *pInfo)
{
	for(int i = 0; i < iCount; i++)
	{
		const auto &aInfoOne = pInfo[i];

		EraseMyEntity(aInfoOne.m_pEntity->m_pInstance);
	}
}

void EntityManagerPlugin::OnGSFactoryCSpawnGroupMgrGameSystemSetGlobalStrHook(void *pValue)
{
	if(m_aLogger.IsChannelEnabled(LV_DETAILED))
	{
		m_aLogger.DetailedFormat("EntityManagerPlugin::OnGSFactoryCSpawnGroupMgrGameSystemSetGlobalStrHook(%p)\n", pValue);
	}

	if(pValue)
	{
		if(!g_pSpawnGroupMgr)
		{
			InitSpawnGroup(reinterpret_cast<CSpawnGroupMgrGameSystem *>(pValue));
		}
	}
	else
	{
		DestroySpawnGroup();
		g_pSpawnGroupMgr = NULL;
	}
}

void EntityManagerPlugin::OnAllocateSpawnGroupHook(SpawnGroupHandle_t hSpawnGroup, ISpawnGroup *pSpawnGroup)
{
	if(m_aLogger.IsChannelEnabled(LV_DETAILED))
	{
		m_aLogger.DetailedFormat("EntityManagerPlugin::OnAllocateSpawnGroupHook(%d, %p)\n", hSpawnGroup, pSpawnGroup);
	}

	// Load settings by spawn group name.
	{
		char sSettingsError[256];

		if(!LoadSettings(pSpawnGroup, (char *)sSettingsError, sizeof(sSettingsError)))
		{
			m_aLogger.WarningFormat(LOGGER_COLOR_WARNING, "Failed to load a settings: %s\n", sSettingsError);
		}
	}

	s_aEntityManagerProviderAgent.OnSpawnGroupAllocated(hSpawnGroup, pSpawnGroup);
}

void EntityManagerPlugin::OnSpawnGroupInitHook(SpawnGroupHandle_t hSpawnGroup, IEntityResourceManifest *pManifest, IEntityPrecacheConfiguration *pConfig, ISpawnGroupPrerequisiteRegistry *pRegistry)
{
	if(m_aLogger.IsChannelEnabled(LV_DETAILED))
	{
		m_aLogger.DetailedFormat("EntityManagerPlugin::OnSpawnGroupInitHook(%d, %p, %p, %p)\n", hSpawnGroup, pManifest, pConfig, pRegistry);
	}

	s_aEntityManagerProviderAgent.OnSpawnGroupInit(hSpawnGroup, pManifest, pConfig, pRegistry);
}

ILoadingSpawnGroup *EntityManagerPlugin::OnCreateLoadingSpawnGroupHook(SpawnGroupHandle_t hSpawnGroup, bool bSynchronouslySpawnEntities, bool bConfirmResourcesLoaded, const CUtlVector<const CEntityKeyValues *> *pKeyValues)
{
	m_aLogger.DetailedFormat("EntityManagerPlugin::CreateLoadingSpawnGroup(%d, bSynchronouslySpawnEntities = %s, bConfirmResourcesLoaded = %s, pKeyValues = %p)\n", hSpawnGroup, bSynchronouslySpawnEntities ? "true" : "false", bConfirmResourcesLoaded ? "true" : "false", pKeyValues);

	auto funcCreateLoadingSpawnGroup = &CSpawnGroupMgrGameSystem::CreateLoadingSpawnGroup;

	SET_META_RESULT(MRES_HANDLED);
	SH_GLOB_SHPTR->DoRecall();

	auto *pSpawnGroupMgr = reinterpret_cast<CSpawnGroupMgrGameSystem *>(SourceHook::RecallGetIface(SH_GLOB_SHPTR, funcCreateLoadingSpawnGroup));

	auto aSpawnGroupMgr = EntityManager::CSpawnGroupAccess(pSpawnGroupMgr);

	CMapSpawnGroup *pMapSpawnGroup = aSpawnGroupMgr.Get(hSpawnGroup);

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

			SpawnGroupHandle_t hTargetSpawnGroup = bIsMySpawnGroup ? pMapSpawnGroup->GetOwnerSpawnGroup() : hSpawnGroup;

			bool bHasInSpawnQueue = s_aEntityManagerProviderAgent.HasInSpawnQueue(hTargetSpawnGroup);

			if(bHasInSpawnQueue)
			{
				s_aEntityManagerProviderAgent.CopySpawnQueueWithEntitySystemOwnership(vecLayerEntities, hTargetSpawnGroup);
			}

			int iOldEntitiesCount = vecLayerEntities.Count();

			s_aEntityManagerProviderAgent.OnSpawnGroupCreateLoading(hSpawnGroup, pMapSpawnGroup, bSynchronouslySpawnEntities, bConfirmResourcesLoaded, vecLayerEntities);

			if(bHasInSpawnQueue || iOldEntitiesCount != vecLayerEntities.Count())
			{
				pKeyValues = &vecLayerEntities;
			}
		}
	}

	ILoadingSpawnGroup *pLoading = (pSpawnGroupMgr->*(funcCreateLoadingSpawnGroup))(hSpawnGroup, bSynchronouslySpawnEntities, bConfirmResourcesLoaded, pKeyValues);

	const int iCount = pLoading->EntityCount();

	if(iCount)
	{
#ifdef _DEBUG
		auto aDetails = m_aLogger.CreateDetailsScope();
#endif

		CUtlVector<EntitySpawnInfo_t> aMyEntities;

		{
			int i = 0;

			const EntitySpawnInfo_t *pEntities = pLoading->GetEntities();

#ifdef _DEBUG
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

				const CEntityKeyValues *pEntityKeyValues = aEntity.m_pKeyValues;

				bool bInSpawnQueue = s_aEntityManagerProviderAgent.HasInSpawnQueue(pEntityKeyValues);

#ifdef _DEBUG
				aDetails.Push(pszMyEntityMessages[bInSpawnQueue]);

				auto aEntityDetails = Logger::Scope(LOGGER_COLOR_ENTITY_KV3, "\t");

				if(s_aEntityManagerProviderAgent.DumpEntityKeyValues(pEntityKeyValues, EntityManager::ProviderAgent::s_eDefaultDEKVFlags, aEntityDetails, &aEntityDetails))
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

#ifdef _DEBUG
		aDetails.SendColor([this](Color rgba, const CUtlString &sContext)
		{
			m_aLogger.Detailed(rgba, sContext);
		});
#endif

		if(!m_bIsCurrentMySpawnOfEntities)
		{
			ListenLoadingSpawnGroup(hSpawnGroup, aMyEntities.Count(), aMyEntities.Base());
		}
	}

	RETURN_META_VALUE(MRES_SUPERCEDE, pLoading);
}

void EntityManagerPlugin::OnSpawnGroupShutdownHook(SpawnGroupHandle_t hSpawnGroup)
{
	if(m_aLogger.IsChannelEnabled(LV_DETAILED))
	{
		m_aLogger.DetailedFormat("EntityManagerPlugin::OnSpawnGroupShutdownHook(%d)\n", hSpawnGroup);
	}

	s_aEntityManagerProviderAgent.OnSpawnGroupDestroyed(hSpawnGroup);
}

void EntityManagerPlugin::ListenLoadingSpawnGroup(SpawnGroupHandle_t hSpawnGroup, const int iCount, const EntitySpawnInfo_t *pEntities, CEntityInstance *pListener)
{
	if(m_aLogger.IsChannelEnabled(LV_DETAILED))
	{
		m_aLogger.DetailedFormat("EntityManagerPlugin::ListenLoadingSpawnGroup(%d, %d, %p, %p)\n", hSpawnGroup, iCount, pEntities, pListener);
	}

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
	if(m_aLogger.IsChannelEnabled(LV_DETAILED))
	{
		m_aLogger.DetailedFormat("EntityManagerPlugin::OnMyEntityFinish(%s (%d))\n", pEntity->GetClassname(), pEntity->GetEntityIndex().Get());
	}

	m_vecMyEntities.push_back(pEntity);
}

EntityManagerPlugin::BaseEvent::BaseEvent(const char *pszName)
 :   m_pszName(pszName)
{
}

bool EntityManagerPlugin::BaseEvent::Init(char *psError, size_t nMaxLength)
{
	const char *pszMyName = GetName();

	bool bResult = gameeventmanager->AddListener(this, pszMyName, true);

	if(!bResult)
	{
		snprintf(psError, nMaxLength, "Failed to add \"%s\" event to listener", pszMyName);
	}

	return bResult;
}

void EntityManagerPlugin::BaseEvent::Destroy()
{
	if(gameeventmanager)
	{
		gameeventmanager->RemoveListener(this);
	}
}

const char *EntityManagerPlugin::BaseEvent::GetName()
{
	return m_pszName;
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
