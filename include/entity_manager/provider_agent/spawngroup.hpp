#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_SPAWNGROUP_HPP_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_SPAWNGROUP_HPP_

#include <gamesystems/spawngroup_manager.h>
#include <worldrenderer/icomputeworldorigin.h>

#define INVALID_SPAWN_GROUP ((SpawnGroupHandle_t)-1)

namespace EntityManager
{
	class ISpawnGroupNotifications
	{
	public:
		virtual void NotifyAllocateSpawnGroup(SpawnGroupHandle_t handle, ISpawnGroup *pSpawnGroup) = 0;
		virtual void NotifyDestroySpawnGroup(SpawnGroupHandle_t handle) = 0;
	};

	class SpawnGroup : public ISpawnGroupNotifications, public IComputeWorldOriginCallback
	{
	public:
		~SpawnGroup();

		int GetStatus() const;
		static bool IsResidentOrStreaming(SpawnGroupHandle_t hSpawnGroup);
		SpawnGroupHandle_t GetAllocatedSpawnGroup() const;
		const char *GetLevelName() const;
		const char *GetLandmarkName() const;
		const Vector &GetLandmarkOffset() const;

		bool Start(const SpawnGroupDesc_t &aDesc, const Vector &vecLandmarkOffset);
		bool Unload();

	public:
		void NotifyAllocateSpawnGroup(SpawnGroupHandle_t handle, ISpawnGroup *pSpawnGroup);
		void NotifyDestroySpawnGroup(SpawnGroupHandle_t handle);

	public: // IComputeWorldOriginCallback
		matrix3x4_t ComputeWorldOrigin(const char *pWorldName, SpawnGroupHandle_t hSpawnGroup, IWorld *pWorld);

	private:
		SpawnGroupHandle_t m_hSpawnGroup = INVALID_SPAWN_GROUP;
		CUtlString m_sLevelName;
		CUtlString m_sLandmarkName;
		Vector m_vecLandmarkOffset;
	};
};

#endif // _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_SPAWNGROUP_HPP_
