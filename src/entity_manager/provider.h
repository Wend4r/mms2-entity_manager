#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_H_

#include <stddef.h>
#include <stdint.h>

#include <tier0/dbg.h>
#include <tier0/platform.h>
#include <tier0/utlscratchmemory.h>
#include <tier1/utldelegateimpl.h>
#include <entity2/entitykeyvalues.h>

#include "provider/entitysystem.h"

#include "gamedata.hpp"

class CSpawnGroupMgrGameSystem;

namespace EntityManager
{
	class Provider
	{
	public:
		bool Init(char *psError, size_t nMaxLength);
		bool Load(const char *pszBaseDir, char *psError = NULL, size_t nMaxLength = 0);
		void Destroy();

	protected:
		bool LoadGameData(const char *pszBaseDir, char *psError = NULL, size_t nMaxLength = 0);

	public:
		class GameDataStorage
		{
		public:
			bool Load(IGameData *pRoot, const char *pszBaseConfigDir, char *psError = NULL, size_t nMaxLength = 0);

		protected:
			bool LoadEntitySystem(IGameData *pRoot, KeyValues *pGameConfig, char *psError = NULL, size_t nMaxLength = 0);
			bool LoadGameResource(IGameData *pRoot, KeyValues *pGameConfig, char *psError = NULL, size_t nMaxLength = 0);
			bool LoadSource2Server(IGameData *pRoot, KeyValues *pGameConfig, char *psError = NULL, size_t nMaxLength = 0);
			bool LoadEntitySpawnGroup(IGameData *pRoot, KeyValues *pGameConfig, char *psError = NULL, size_t nMaxLength = 0);

		public:
			class EntitySystem
			{
			public:
				EntitySystem();

			public:
				bool Load(IGameData *pRoot, KeyValues *pGameConfig, char *psError = NULL, size_t nMaxLength = 0);
				void Reset();

			public:
				typedef CEntityInstance *(*OnCreateEntityPtr)(CEntitySystem * const pThis, SpawnGroupHandle_t hSpawnGroup, const char *pszNameOrDesignName, EntityNetworkingMode_t eNetworkMode, CEntityIndex iForcedIndex, int iForcedSerial, bool bCreateInIsolatedPrecacheList);
				typedef void (*OnQueueSpawnEntityPtr)(CEntitySystem * const pThis, CEntityIdentity *pEntity, const CEntityKeyValues *pInitializationData);
				typedef void (*OnQueueDestroyEntityPtr)(CEntitySystem * const pThis, CEntityIdentity *pEntity);
				typedef void (*OnExecuteQueuedCreationPtr)(CEntitySystem * const pThis);
				typedef void (*OnExecuteQueuedDeletionPtr)(CEntitySystem * const pThis, bool bPerformDeallocation);
				typedef void (*OnListenForEntityInSpawnGroupToFinishPtr)(CGameEntitySystem * const pThis, SpawnGroupHandle_t hSpawnGroup, CEntityInstance *pEntityToListenFor, const CEntityKeyValues *pKeyValues, CEntityInstance *pListener, CUtlDelegate<void (CEntityInstance *, const CEntityKeyValues *)> handler);

				OnCreateEntityPtr CreateEntityFunction() const;
				OnQueueSpawnEntityPtr QueueSpawnEntityFunction() const;
				OnQueueDestroyEntityPtr QueueDestroyEntityFunction() const;
				OnExecuteQueuedCreationPtr ExecuteQueuedCreationFunction() const;
				OnExecuteQueuedDeletionPtr ExecuteQueuedDeletionFunction() const;
				OnListenForEntityInSpawnGroupToFinishPtr ListenForEntityInSpawnGroupToFinishFunction() const;

				ptrdiff_t GetCurrentManifestOffset() const;
				ptrdiff_t GetKeyValuesContextAllocatorOffset() const;

			private:
				GameData::Config::Addresses::ListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config::Offsets::ListenerCallbacksCollector m_aOffsetCallbacks;
				GameData::Config m_aGameConfig;

			private: // Signatures.
				OnCreateEntityPtr m_pfnCreateEntity = nullptr;
				OnQueueSpawnEntityPtr m_pfnQueueSpawnEntity = nullptr;
				OnQueueDestroyEntityPtr m_pfnQueueDestroyEntity = nullptr;
				OnExecuteQueuedCreationPtr m_pfnExecuteQueuedCreation = nullptr;
				OnExecuteQueuedDeletionPtr m_pfnExecuteQueuedDeletion = nullptr;
				OnListenForEntityInSpawnGroupToFinishPtr m_pfnListenForEntityInSpawnGroupToFinish = nullptr;

			private: // Offsets.
				ptrdiff_t m_nCurrentManifestOffset = -1;
				ptrdiff_t m_nEntityKeyValuesAllocatorOffset = -1;
			};

			class GameResource
			{
			public:
				GameResource();

			public:
				bool Load(IGameData *pRoot, KeyValues *pGameConfig, char *psError = NULL, size_t nMaxLength = 0);
				void Reset();

			public:
				ptrdiff_t GetDestroyResourceManifestOffset() const;
				ptrdiff_t GetPrecacheEntitiesAndConfirmResourcesAreLoadedOffset() const;
				ptrdiff_t GetAllocGameResourceManifestOffset() const;
				ptrdiff_t GetAppendToAndCreateGameResourceManifestOffset() const;
				ptrdiff_t GetEntitySystemOffset() const;
				ptrdiff_t GetEntityManifestOffset() const;
				ptrdiff_t GetEntityManifestVFTableOffset() const;

			private:
				// GameData::Config::Addresses::ListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config::Offsets::ListenerCallbacksCollector m_aOffsetCallbacks;
				GameData::Config m_aGameConfig;

			private: // Offsets.
				ptrdiff_t m_nDestroyResourceManifestOffset = -1;
				ptrdiff_t m_nPrecacheEntitiesAndConfirmResourcesAreLoadedOffset = -1;
				ptrdiff_t m_nAllocGameResourceManifestOffset = -1;
				ptrdiff_t m_nAppendToAndCreateGameResourceManifestOffset = -1;
				ptrdiff_t m_nEntitySystemOffset = -1;
				ptrdiff_t m_nEntityManifestOffset = -1;
				ptrdiff_t m_nEntityManifestVFTableOffset = -1;
			};

			class Source2Server
			{
			public:
				Source2Server();

			public:
				bool Load(IGameData *pRoot, KeyValues *pGameConfig, char *psError = NULL, size_t nMaxLength = 0);
				void Reset();

			public:
				ptrdiff_t GetGameEventManagerOffset() const;

			private:
				// GameData::Config::Addresses::ListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config::Offsets::ListenerCallbacksCollector m_aOffsetCallbacks;
				GameData::Config m_aGameConfig;

			private: // Offsets.
				ptrdiff_t m_nGetterGameEventManager = -1;
			};

			class SpawnGroup
			{
			public:
				SpawnGroup();

			public:
				bool Load(IGameData *pRoot, KeyValues *pGameConfig, char *psError = NULL, size_t nMaxLength = 0);
				void Reset();

			public:
				CSpawnGroupMgrGameSystem **GetSpawnGroupMgrAddress() const;
				ptrdiff_t GetMgrGameSystemSpawnGroupsOffset() const;
				ptrdiff_t GetLoadingMapSpawnInfoOffset() const;
				ptrdiff_t GetBaseSpawnGroupEntityFilterNameOffset() const;

			private:
				CSpawnGroupMgrGameSystem **m_ppSpawnGroupMgrAddress = nullptr;
				ptrdiff_t m_nMgrGameSystemSpawnGroupsOffset = -1;
				ptrdiff_t m_nLoadingMapSpawnInfoOffset = -1;
				ptrdiff_t m_nBaseSpawnGroupEntityFilterNameOffset = -1;

			private:
				GameData::Config::Addresses::ListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config::Offsets::ListenerCallbacksCollector m_aOffsetCallbacks;
				GameData::Config m_aGameConfig;
			};

			const EntitySystem &GetEntitySystem() const;
			const GameResource &GetGameResource() const;
			const Source2Server &GetSource2Server() const;
			const SpawnGroup &GetSpawnGroup() const;

		private:
			EntitySystem m_aEntitySystem;
			GameResource m_aGameResource;
			Source2Server m_aSource2Server;
			SpawnGroup m_aSpawnGroup;
		};

		const GameDataStorage &GetGameDataStorage() const;

	private:
		GameData m_aData;
		GameDataStorage m_aStorage;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_H_
