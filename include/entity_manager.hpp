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

#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_HPP_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_HPP_

#include <ientitymgr.hpp>
#include <entity_manager/provider.hpp>
#include <entity_manager/provider_agent.hpp>
#include <entity_manager/settings.hpp>

#include <ISmmPlugin.h>
#include <igameevents.h>
#include <iplayerinfo.h>
#include <sh_vector.h>

#include <tier1/utlvector.h>
#include <entity2/entityidentity.h>

#include "logger.hpp"

#define PREFIX_ENTITY_MANAGER META_PLUGIN_PREFIX

class ISpawnGroup;
class ISpawnGroupPrerequisiteRegistry;

class EntityManagerPlugin final : public ISmmPlugin, public IMetamodListener, public IEntityManager
{
public:
	EntityManagerPlugin();

public:
	using IProviderAgent = IEntityManager::IProviderAgent;

public: // ISmmPlugin
	bool Load(PluginId id, ISmmAPI *ismm, char *error, size_t maxlen, bool late) override;
	bool Unload(char *error, size_t maxlen) override;
	bool Pause(char *error, size_t maxlen) override;
	bool Unpause(char *error, size_t maxlen) override;
	void AllPluginsLoaded() override;

public: // ISmmPlugin
	const char *GetAuthor() override;
	const char *GetName() override;
	const char *GetDescription() override;
	const char *GetURL() override;
	const char *GetLicense() override;
	const char *GetVersion() override;
	const char *GetDate() override;
	const char *GetLogTag() override;

public: // IMetamodListener
	void *OnMetamodQuery(const char *iface, int *ret) override;

public: // IEntityManager
	IProviderAgent *GetProviderAgent() override;
	CSpawnGroupMgrGameSystem *GetSpawnGroupManager() override;

protected:
	bool InitEntitySystem();
	void DestroyEntitySystem();

	bool InitGameEvents();
	void DestroyGameEvents();

	bool InitGameResource();
	void DestroyGameResource();

	bool InitGameSystem();
	void DestroyGameSystem();

	bool InitSpawnGroup();
	void DestroySpawnGroup();

	bool HookEvents(char *psError = NULL, size_t nMaxLength = 0);
	void UnhookEvents();

protected:
	bool LoadProvider();
	bool LoadSettings(ISpawnGroup *pSpawnGroup, char *psError = NULL, size_t nMaxLength = 0);
	virtual void OnBasePathChanged(const char *pszNewOne);

public:
	void SpawnMyEntities();
	void DestroyMyEntities();

protected:
	bool EraseMyEntity(CEntityInstance *pEntityInst);

private: // Commands.
	CON_COMMAND_MEMBER_F(EntityManagerPlugin, "mm_" PREFIX_ENTITY_MANAGER "_set_basepath", OnSetBasePathCommand, "Set base path for Entity Manager", FCVAR_LINKED_CONCOMMAND);

public: // SourceHooks.
	void OnStartupServerHook(const GameSessionConfiguration_t &config, ISource2WorldSession *pWorldSession, const char *);
	int OnLoadEventsFromFileHook(const char *pszFilename, bool bSearchAll);
	void OnEntitySystemSpawnHook(int iCount, const EntitySpawnInfo_t *pInfo);
	void OnEntitySystemUpdateOnRemoveHook(int iCount, const EntityDeletion_t *pInfo);
	void OnGSFactoryCSpawnGroupMgrGameSystemSetGlobalStrHook(void *pValue);
	void OnAllocateSpawnGroupHook(SpawnGroupHandle_t handle, ISpawnGroup *pSpawnGroup);
	void OnSpawnGroupInitHook(SpawnGroupHandle_t handle, IEntityResourceManifest *pManifest, IEntityPrecacheConfiguration *pConfig, ISpawnGroupPrerequisiteRegistry *pRegistry);
	ILoadingSpawnGroup *OnCreateLoadingSpawnGroupHook(SpawnGroupHandle_t handle, bool bSynchronouslySpawnEntities, bool bConfirmResourcesLoaded, const CUtlVector<const CEntityKeyValues *> *pKeyValues);
	void OnSpawnGroupShutdownHook(SpawnGroupHandle_t handle);

public:
	void ListenLoadingSpawnGroup(SpawnGroupHandle_t hSpawnGroup, const int iCount, const EntitySpawnInfo_t *pEntities, CEntityInstance *pListener = NULL);
	void OnMyEntityFinish(CEntityInstance *pEntity, const CEntityKeyValues *pKeyValues);

private:
	class BaseEvent : public IGameEventListener2
	{
	public:
		BaseEvent(const char *pszName);

		bool Init(char *psError = NULL, size_t nMaxLength = 0);
		void Destroy();

		const char *GetName();

	private:
		const char *m_pszName;
	};

	class RoundPreStartEvent : public BaseEvent
	{
	public:
		RoundPreStartEvent();

		void FireGameEvent(IGameEvent *pEvent);
	};

	class RoundStartEvent : public BaseEvent
	{
	public:
		RoundStartEvent();

		void FireGameEvent(IGameEvent *pEvent);
	};

private:
	std::string m_sBasePath = "addons" CORRECT_PATH_SEPARATOR_S META_PLUGIN_PREFIX;
	std::string m_sCurrentMap = "\0";

	bool m_bIsCurrentMySpawnOfEntities = false;
	std::vector<CEntityInstance *> m_vecMyEntities;

	bool m_bIsHookedEvents = false;
	RoundPreStartEvent m_aRoundPreStart;
	RoundStartEvent m_aRoundStart;

	EntityManager::Settings m_aSettings;
	Logger m_aLogger;

	int m_iLoadEventsFromFileId;
};

DLL_IMPORT EntityManagerPlugin *g_pEntityManager;
DLL_IMPORT EntityManager::Provider *g_pEntityManagerProvider;
DLL_IMPORT EntityManager::ProviderAgent *g_pEntityManagerProviderAgent;

PLUGIN_GLOBALVARS();

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_HPP_
