#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_HPP_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_HPP_

#include <stddef.h>
#include <stdint.h>

#include <tier0/dbg.h>
#include <tier0/platform.h>
#include <tier0/utlscratchmemory.h>
#include <tier1/utldelegateimpl.h>
#include <entity2/entitykeyvalues.h>

#include "provider/entitysystem.hpp"

#include <gamedata.hpp>

class CSpawnGroupMgrGameSystem;
class CBaseGameSystemFactory;
class CGameEventManager;

namespace EntityManager
{
	class Provider : public IGameData
	{
	public:
		Provider();

		bool Init(GameData::CStringVector &vecMessages);
		bool Load(const char *pszBaseDir, GameData::CStringVector &vecMessages);
		void Destroy();

	public: // IGameData
		const DynLibUtils::CModule *FindLibrary(const char *pszName) const;

	public:
		CUtlSymbolLarge GetSymbol(const char *pszText);
		CUtlSymbolLarge FindSymbol(const char *pszText) const;

	protected:
		bool LoadGameData(const char *pszBaseDir, GameData::CStringVector &vecMessages);

	public:
		class GameDataStorage
		{
		public:
			bool Load(IGameData *pRoot, const char *pszBaseConfigDir, GameData::CStringVector &vecMessages);

		protected:
			bool LoadEntityResourceManifest(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
			bool LoadEntitySystem(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
			bool LoadGameResource(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
			bool LoadGameSystem(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
			bool LoadSource2Server(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
			bool LoadEntitySpawnGroup(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);

		public:
			class EntityResourceManifest
			{
			public:
				EntityResourceManifest();

			public:
				bool Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
				void Reset();

			public:
				void AddResource(IEntityResourceManifest *pEntityManifest, const char *pszPath) const;

			private:
				GameData::Config::Offsets_t::CListenerCallbacksCollector m_aOffsetCallbacks;
				GameData::Config m_aGameConfig;

			private: // Offsets.
				ptrdiff_t m_nAddResourceIndex = -1;
			};

			class EntitySystem
			{
			public:
				EntitySystem();

			public:
				bool Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
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
				GameData::Config::Addresses_t::CListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config::Offsets_t::CListenerCallbacksCollector m_aOffsetCallbacks;
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
				bool Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
				void Reset();

			public:
				ptrdiff_t GetDestroyResourceManifestOffset() const;
				ptrdiff_t GetPrecacheEntitiesAndConfirmResourcesAreLoadedOffset() const;
				ptrdiff_t GetAllocGameResourceManifestOffset() const;
				ptrdiff_t GetAppendToAndCreateGameResourceManifestOffset() const;
				ptrdiff_t GetEntitySystemOffset() const;
				ptrdiff_t GetResouceManifestOffset() const;
				ptrdiff_t GetResouceManifestVFTableOffset() const;

			private:
				// GameData::Config::Addresses_t::CListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config::Offsets_t::CListenerCallbacksCollector m_aOffsetCallbacks;
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

			class GameSystem
			{
			public:
				GameSystem();

			public:
				bool Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
				void Reset();

			public:
				typedef CBaseGameSystemFactory **(*OnGameSystemInitPtr)();

				CBaseGameSystemFactory **GetBaseGameSystemFactoryFirst() const;
				OnGameSystemInitPtr GameSystemInitFunction() const;

			private:
				GameData::Config::Addresses_t::CListenerCallbacksCollector m_aAddressCallbacks;
				// GameData::Config::Offsets_t::CListenerCallbacksCollector m_aOffsetCallbacks;
				GameData::Config m_aGameConfig;

			private: // Addresses.
				CBaseGameSystemFactory **m_pBaseGameSystemFactoryFirst = nullptr;
				OnGameSystemInitPtr m_pfnGameSystemInit = nullptr;
			};

			class Source2Server
			{
			public:
				Source2Server();

			public:
				bool Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
				void Reset();

			public:
				CGameEventManager **GetGameEventManagerPtr() const;
				ptrdiff_t GetGameEventManagerOffset() const;

			private:
				GameData::Config::Addresses_t::CListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config::Offsets_t::CListenerCallbacksCollector m_aOffsetCallbacks;
				GameData::Config m_aGameConfig;

			private: // Addresses.
				CGameEventManager **m_ppGameEventManager = nullptr;

			private: // Offsets.
				ptrdiff_t m_nGetterGameEventManager = -1;
			};

			class SpawnGroup
			{
			public:
				SpawnGroup();

			public:
				bool Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages);
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
				GameData::Config::Addresses_t::CListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config::Offsets_t::CListenerCallbacksCollector m_aOffsetCallbacks;
				GameData::Config m_aGameConfig;
			};

			const EntityResourceManifest &GetEntityResourceManifest() const;
			const EntitySystem &GetEntitySystem() const;
			const GameResource &GetGameResource() const;
			const GameSystem &GetGameSystem() const;
			const Source2Server &GetSource2Server() const;
			const SpawnGroup &GetSpawnGroup() const;

		private:
			EntityResourceManifest m_aEntityResourceManifest;
			EntitySystem m_aEntitySystem;
			GameResource m_aGameResource;
			GameSystem m_aGameSystem;
			Source2Server m_aSource2Server;
			SpawnGroup m_aSpawnGroup;
		};

		const GameDataStorage &GetGameDataStorage() const;

	private:
		CUtlSymbolTableLarge_CI m_aSymbolTable;

		CUtlMap<CUtlSymbolLarge, DynLibUtils::CModule *> m_mapLibraries;
		GameDataStorage m_aStorage;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_HPP_
