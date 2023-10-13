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
#include <sh_vector.h>

#include "entity_manager/gamedata.h"
#include "entity_manager/provider.h"
#include "entity_manager/provider_agent.h"
#include "entity_manager/settings.h"

#define PREFIX_ENTITY_MANAGER META_PLUGIN_NAME

class CGameResourceService;

class EntityManager final : public ISmmPlugin, public IMetamodListener
{
public:
	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late);
	bool Unload(char *error, size_t maxlen);
	bool Pause(char *error, size_t maxlen);
	bool Unpause(char *error, size_t maxlen);
	void AllPluginsLoaded();

protected:
	void InitEntitySystem();
	void DestroyEntitySystem();

protected:
	bool LoadGameData(char *psError = NULL, size_t nMaxLength = 0);
	bool LoadProvider(char *psError = NULL, size_t nMaxLength = 0);
	bool LoadSettings(char *psError = NULL, size_t nMaxLength = 0);
	virtual void OnBasePathChanged(const char *pszNewOne);

private: // Commands.
	CON_COMMAND_MEMBER_F(EntityManager, "mm_" PREFIX_ENTITY_MANAGER "_set_basepath", OnSetBasePathCommand, "Set base path for Entity Manager", FCVAR_LINKED_CONCOMMAND);

public: // SourceHooks.
	void OnLevelInit(char const *pszMapName, char const *pszMapEntities, char const *pszOldLevel, char const *pszLandmarkName, bool bIsLoadGame, bool bIsBackground);
	void OnGameFrameHook(bool simulating, bool bFirstTick, bool bLastTick);
	void OnStartupServerHook(const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession, const char *pszMapName);
	void OnEntitySystemSpawn(int nCount, const EntitySpawnInfo_t *pInfo);

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

#ifdef PLATFORM_WINDOWS
	std::string m_sBasePath = "addons\\" META_PLUGIN_NAME;
#else
	std::string m_sBasePath = "addons/" META_PLUGIN_NAME;
#endif

	EntityManagerSpace::Settings m_aSettings;
	ptrdiff_t m_nGameResourceServiceEntitySystemOffset = -1;

	std::string m_sCurrentMap;
};

extern EntityManager *g_pEntityManager;
extern EntityManagerSpace::GameData *g_pEntityManagerGameData;
extern EntityManagerSpace::Provider *g_pEntityManagerProvider;
extern EntityManagerSpace::ProviderAgent *g_pEntityManagerProviderAgent;

PLUGIN_GLOBALVARS();

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_H_
