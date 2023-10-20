#ifndef SPAWNGROUP_MANAGER_H
#define SPAWNGROUP_MANAGER_H

#include "entity2/entityidentity.h"
#include "tier1/utlstring.h"
#include "tier1/utlvector.h"

class CEntityKeyValues;
class ISpawnGroup;
class ISpawnGroupPrerequisiteRegistry;

struct SaveRestoreDataHandle_t__
{
	int unused;
};
typedef SaveRestoreDataHandle_t__ *SaveRestoreDataHandle_t;

struct EventGameInit_t;

struct EventPreSpawnGroupLoad_t
{
	CUtlString m_SpawnGroupName;
	CUtlString m_EntityLumpName;
	SpawnGroupHandle_t m_SpawnGroupHandle;
};

struct EventServerPostEntityThink_t
{
	bool m_bFirstTick;
	bool m_bLastTick;
};

class IGameSpawnGroupMgr
{
public:
	virtual void AllocateSpawnGroup(SpawnGroupHandle_t h, ISpawnGroup *pSpawnGroup) = 0;
	virtual void ReleaseSpawnGroup(SpawnGroupHandle_t h) = 0;
	virtual void SpawnGroupInit(SpawnGroupHandle_t handle, IEntityResourceManifest *pManifest, IEntityPrecacheConfiguration *pConfig, ISpawnGroupPrerequisiteRegistry *pRegistry) = 0;
	virtual ILoadingSpawnGroup *CreateLoadingSpawnGroup(SpawnGroupHandle_t handle, bool bSynchronouslySpawnEntities, bool bConfirmResourcesLoaded, const CUtlVector<const CEntityKeyValues *> *pKeyValues) = 0;
	virtual ILoadingSpawnGroup *CreateLoadingSpawnGroupForSaveFile(SpawnGroupHandle_t handle, const void *pSaveData, size_t nSaveDataSize) = 0;
	virtual void SpawnGroupSpawnEntities(SpawnGroupHandle_t handle) = 0;
	virtual void SpawnGroupShutdown(SpawnGroupHandle_t handle) = 0;
	virtual void SetActiveSpawnGroup(SpawnGroupHandle_t handle) = 0;
	virtual void UnloadSpawnGroup(SpawnGroupHandle_t hSpawnGroup, bool bSaveEntities) = 0;
	virtual SpawnGroupHandle_t GetParentSpawnGroup(SpawnGroupHandle_t hSpawnGroup) = 0;
	virtual int GetChildSpawnGroupCount(SpawnGroupHandle_t handle) = 0;
	virtual SaveRestoreDataHandle_t SaveGame_Start(const char *pSaveName, const char *pOldLevel, const char *pszLandmarkName) = 0;
	virtual bool SaveGame_Finalize(SaveRestoreDataHandle_t hSaveRestore) = 0;
	virtual bool FrameUpdatePostEntityThink(const EventServerPostEntityThink_t &msg) = 0;
	virtual const char *SaveGame_GetLastSaveFile(SaveRestoreDataHandle_t hSaveRestore) = 0;
	virtual bool IsGameReadyToSave() = 0;
	virtual unsigned long UnkGetGlobal1() = 0;
	virtual void * const UnkGetGameEntitySystemSync1() = 0;
	virtual void UnkRelease1() = 0;
	virtual void UnkRelease2() = 0;
	virtual void YouForgot_DECLARE_GAME_SYSTEM_InYourClassDefinition() = 0;
	virtual void UnkSpawnGroupManagerGameSystemMember1();
	virtual void GameInit(const EventGameInit_t &msg) = 0;
	virtual void GameShutdown(const EventGameInit_t &msg) = 0;
	virtual void FrameBoundary(const EventGameInit_t &msg) = 0;
	virtual void PreSpawnGroupLoad(const EventPreSpawnGroupLoad_t &msg) = 0;

	// IGameSystem is at next one.
};

class CSpawnGroupMgrGameSystem : public IGameSpawnGroupMgr //, public IGameSystem
{
	virtual ~CSpawnGroupMgrGameSystem() = 0;
};

#endif // SPAWNGROUP_MANAGER_H
