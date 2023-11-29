#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_SPAWNGROUP_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_SPAWNGROUP_

#include <gamesystems/spawngroup_manager.h>
#include <worldrenderer/icomputeworldorigin.h>

#define INVALID_SPAWN_GROUP ((SpawnGroupHandle_t)-1)

namespace EntityManager
{
	class SpawnGroup : public IComputeWorldOriginCallback
	{
	public:
		~SpawnGroup();

		int GetStatus();
		static bool IsResidentOrStreaming(SpawnGroupHandle_t hSpawnGroup);
		const Vector &GetLandmarkOffset();

		bool Start(const char *pLevelName, const char *pEntityLumpName = NULL, const char *pSpawnGroupFilterName = NULL, const char *pWorldGroupname = NULL, float flTimeoutInterval = 0.0f, const char *pLandmarkName = NULL, SpawnGroupHandle_t hOwner = INVALID_SPAWN_GROUP);

	public: // IComputeWorldOriginCallback
		matrix3x4_t ComputeWorldOrigin(const char *pWorldName, SpawnGroupHandle_t hSpawnGroup, IWorld *pWorld);

	private:
		SpawnGroupHandle_t m_hSpawnGroup = INVALID_SPAWN_GROUP;
		CUtlString m_sLevelName;
		CUtlString m_sLandmarkName;
		Vector m_vecLandmarkOffset;
	};
};

#endif // _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_SPAWNGROUP_
