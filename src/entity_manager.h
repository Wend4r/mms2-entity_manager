/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * Metamod:Source Entity Manager
 * Written by Wend4r.
 * ======================================================
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from 
 * the use of this software.
 */

#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_H_

#include <ISmmPlugin.h>
#include <igameevents.h>
#include <iplayerinfo.h>
#include <metamod_oslink.h>
#include <sh_vector.h>

#include <tier1/utlvector.h>
#include <entity2/entityidentity.h>

#include "entity_manager/provider.h"
#include "entity_manager/provider_agent.h"
#include "entity_manager/settings.h"
#include "logger.h"

#define PREFIX_ENTITY_MANAGER META_PLUGIN_NAME

class ISpawnGroup;
class ISpawnGroupPrerequisiteRegistry;

class EntityManagerPlugin final : public ISmmPlugin, public IMetamodListener
{
public:
	EntityManagerPlugin();

	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late);
	bool Unload(char *error, size_t maxlen);
	bool Pause(char *error, size_t maxlen);
	bool Unpause(char *error, size_t maxlen);
	void AllPluginsLoaded();

protected:
	bool InitGameResource();
	void DestroyGameResource();

	bool InitEntitySystem();
	void DestroyEntitySystem();

	bool InitSpawnGroup();
	void DestroySpawnGroup();

protected:
	bool LoadProvider(char *psError = NULL, size_t nMaxLength = 0);
	bool LoadSettings(ISpawnGroup *pSpawnGroup, char *psError = NULL, size_t nMaxLength = 0);
	virtual void OnBasePathChanged(const char *pszNewOne);

private: // Commands.
	CON_COMMAND_MEMBER_F(EntityManagerPlugin, "mm_" PREFIX_ENTITY_MANAGER "_set_basepath", OnSetBasePathCommand, "Set base path for Entity Manager", FCVAR_LINKED_CONCOMMAND);

public: // SourceHooks.
	void OnGameFrameHook(bool simulating, bool bFirstTick, bool bLastTick);
	void OnStartupServerHook(const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession, const char *);
	void OnAllocateSpawnGroupHook(SpawnGroupHandle_t handle, ISpawnGroup *pSpawnGroup);
	void OnSpawnGroupInitHook(SpawnGroupHandle_t handle, IEntityResourceManifest *pManifest, IEntityPrecacheConfiguration *pConfig, ISpawnGroupPrerequisiteRegistry *pRegistry);
	ILoadingSpawnGroup *OnCreateLoadingSpawnGroupHook(SpawnGroupHandle_t handle, bool bSynchronouslySpawnEntities, bool bConfirmResourcesLoaded, const CUtlVector<const CEntityKeyValues *> *pKeyValues);
	void OnSpawnGroupShutdownHook(SpawnGroupHandle_t handle);

public:
	void ListenLoadingSpawnGroup(SpawnGroupHandle_t hSpawnGroup, const int iCount, const EntitySpawnInfo_t *pEntities, CEntityInstance *pListener = NULL);
	int DestroyMyLoadingSpawnGroupEntities();
	void OnMyEntityFinish(CEntityInstance *pEntity, const CEntityKeyValues *pKeyValues);

public:
	const char *GetAuthor();
	const char *GetName();
	const char *GetDescription();
	const char *GetURL();
	const char *GetLicense();
	const char *GetVersion();
	const char *GetDate();
	const char *GetLogTag();

private:
	std::string m_sBasePath = "addons" PATH_SEP_STR META_PLUGIN_NAME;
	std::string m_sCurrentMap = "\0";

	EntityManager::Settings m_aSettings;
	Logger m_aLogger;

	CUtlVector<CEntityInstance *> m_vecMyEntities;
};

extern EntityManagerPlugin *g_pEntityManager;
extern EntityManager::Provider *g_pEntityManagerProvider;
extern EntityManager::ProviderAgent *g_pEntityManagerProviderAgent;

PLUGIN_GLOBALVARS();

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_H_
