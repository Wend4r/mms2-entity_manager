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
#include <worldrenderer/iworld.h>
#include <worldrenderer/iworldrenderermgr.h>
#include <tier0/dbg.h>
#include <tier1/convar.h>

#include <memory_utils/virtual.h>

#include "entity_manager.h"
#include "entity_manager/provider/gameresource.h"
#include "entity_manager/provider/spawngroup.h"

#define ENTITY_MANAGER_WORLD_ROOT "entity_manager"
#define ENTITY_MANAGER_PROGENITOR_WORLD_NAME "progenitor_layer"

const Color ENTITY_MANAGER_LOGGINING_COLOR = {125, 125, 125, 255};

class GameSessionConfiguration_t { };

SH_DECL_HOOK3_void(IServerGameDLL, GameFrame, SH_NOATTRIB, 0, bool, bool, bool);
SH_DECL_HOOK3_void(INetworkServerService, StartupServer, SH_NOATTRIB, 0, const GameSessionConfiguration_t &, ISource2WorldSession *, const char *);

SH_DECL_HOOK2_void(CGameEntitySystem, Spawn, SH_NOATTRIB, 0, int, const EntitySpawnInfo_t *);

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
	GET_V_IFACE_CURRENT(GetEngineFactory, g_pWorldRendererMgr, IWorldRendererMgr, WORLD_RENDERER_MGR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetFileSystemFactory, filesystem, IFileSystem, FILESYSTEM_INTERFACE_VERSION);
	GET_V_IFACE_ANY(GetServerFactory, server, IServerGameDLL, INTERFACEVERSION_SERVERGAMEDLL);

	// Currently doesn't work from within mm side, use GetGameGlobals() in the mean time instead
	// gpGlobals = ismm->GetCGlobals();

	META_CONPRINTF( "Starting %s plugin.\n", this->GetName());

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
			this->InitGameResource();
			this->InitEntitySystem();
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
						aDesc.m_sEntityFilterName = "mapload";
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
							this->m_aLogger.WarningFormat("Failed to start creating JustInTime spawn group for \"%s\" world\n", pOwnerWorldName);
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
	this->m_aSettings.Destroy();
	s_aEntityManagerProvider.Destroy();
	s_aEntityManagerProviderAgent.Destroy();

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

bool EntityManagerPlugin::InitEntitySystem()
{
	bool bResult = s_aEntityManagerProviderAgent.NotifyEntitySystemUpdated();

	if(bResult)
	{
		SH_ADD_HOOK_MEMFUNC(CGameEntitySystem, Spawn, g_pGameEntitySystem, this, &EntityManagerPlugin::OnEntitySystemSpawnHook, false);
	}

	return bResult;
}

void EntityManagerPlugin::DestroyEntitySystem()
{
	SH_REMOVE_HOOK_MEMFUNC(CGameEntitySystem, Spawn, g_pGameEntitySystem, this, &EntityManagerPlugin::OnEntitySystemSpawnHook, false);
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

void EntityManagerPlugin::OnStartupServerHook(const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession, const char *)
{
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

		this->InitGameResource();
		this->InitEntitySystem();
		this->InitSpawnGroup();
	}
	else
	{
		this->m_aLogger.Warning(LOGGER_COLOR_WARNING, "Failed to get a net server\n");
	}
}

void EntityManagerPlugin::OnEntitySystemSpawnHook(int iCount, const EntitySpawnInfo_t *pInfo)
{
	this->m_aLogger.MessageFormat("EntityManagerPlugin::OnEntitySystemSpawnHook(%d, %p)\n", iCount, pInfo);

#ifdef DEBUG
	auto aDetails = this->m_aLogger.CreateDetailsScope();

	{
		for(int i = 0; i < iCount; i++)
		{
			const EntitySpawnInfo_t &pInfoOne = pInfo[i];

			CEntityIdentity *pEntity = pInfoOne.m_pEntity;

			int iEntity = pEntity->GetEntityIndex().Get();

			const CEntityKeyValues *pKeyValues = pInfoOne.m_pKeyValues;

			aDetails.PushFormat("-- Spawn entity (#%d) --", iEntity);

			auto aEntityDetails = Logger::Scope(LOGGER_COLOR_ENTITY_KV3, "\t");

			if(s_aEntityManagerProviderAgent.DumpEntityKeyValues(pKeyValues, aEntityDetails, &aEntityDetails))
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
		}
	}

	aDetails.SendColor([this](const Color &rgba, const std::string sContent)
	{
		this->m_aLogger.Detailed(rgba, sContent.c_str());
	});
#endif

	SET_META_RESULT(MRES_HANDLED);
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

	if(pMapSpawnGroup)
	{
		if(!V_strncmp(pMapSpawnGroup->GetLocalNameFixup(), ENTITY_MANAGER_WORLD_ROOT, sizeof(ENTITY_MANAGER_WORLD_ROOT) - 1))
		{
			if(pKeyValues)
			{
				vecLayerEntities = *pKeyValues;
				// vecLayerEntities.AddVectorToTail(*pKeyValues);
			}

			s_aEntityManagerProviderAgent.AddSpawnQueueToTail(vecLayerEntities, pMapSpawnGroup->GetOwnerSpawnGroup());
			pKeyValues = &vecLayerEntities;
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

				if(s_aEntityManagerProviderAgent.DumpEntityKeyValues(pKeyValues, aEntityDetails, &aEntityDetails))
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

		this->ListenLoadingSpawnGroup(handle, aMyEntities.Count(), aMyEntities.Base());
	}

	s_aEntityManagerProviderAgent.ReleaseSpawnQueued(handle);

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
