#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_SPAWNGROUP_HPP_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_SPAWNGROUP_HPP_

#include <ientitymgr.hpp>
#include <gamesystems/spawngroup_manager.h>
#include <worldrenderer/icomputeworldorigin.h>

namespace EntityManager
{
	class CSpawnGroupInstance : public IEntityManager::IProviderAgent::ISpawnGroupInstance, public IComputeWorldOriginCallback
	{
	public:
		~CSpawnGroupInstance() override;

	public:
		static bool IsResidentOrStreaming(SpawnGroupHandle_t hSpawnGroup);

	public: // IEntityManager::IProviderAgent::ISpawnGroupLoader
		bool Load(const SpawnGroupDesc_t &aDesc, const Vector &vecLandmarkOffset) override;
		bool Unload() override;

	public: // IEntityManager::IProviderAgent::ISpawnGroupNotifications
		void OnSpawnGroupAllocated(SpawnGroupHandle_t handle, ISpawnGroup *pSpawnGroup) override;
		void OnSpawnGroupDestroyed(SpawnGroupHandle_t handle) override;

	public: // IEntityManager::IProviderAgent::ISpawnGroupCallbacks
		void AddNotificationsListener(ISpawnGroupNotifications *pNotifications) override;
		bool RemoveNotificationsListener(ISpawnGroupNotifications *pNotifications) override;

	public: // IEntityManager::IProviderAgent::ISpawnGroupInstance
		int GetStatus() const override;
		ISpawnGroup *GetSpawnGroup() const override;
		SpawnGroupHandle_t GetSpawnGroupHandle() const override;
		const char *GetLevelName() const override;
		const char *GetLandmarkName() const override;
		const Vector &GetLandmarkOffset() const override;

	public: // IComputeWorldOriginCallback
		matrix3x4_t ComputeWorldOrigin(const char *pWorldName, SpawnGroupHandle_t hSpawnGroup, IWorld *pWorld) override;

	private:
		ISpawnGroup *m_pSpawnGroup = NULL;
		SpawnGroupHandle_t m_hSpawnGroup = INVALID_SPAWN_GROUP;
		CUtlString m_sLevelName;
		CUtlString m_sLandmarkName;
		Vector m_vecLandmarkOffset;

	private:
		CUtlVector<ISpawnGroupNotifications *> m_vecNotificationsCallbacks;
	};
};

#endif // _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_SPAWNGROUP_HPP_
