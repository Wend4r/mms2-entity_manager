#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_H_

#include <stddef.h>
#include <stdint.h>

#include <tier0/dbg.h>
#include <tier0/platform.h>
#include <tier1/utlscratchmemory.h>

#include "provider/entitykeyvalues.h"
#include "provider/entitysystem.h"

#include "gamedata.h"

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
			bool LoadEntityKeyValues(IGameData *pRoot, KeyValues *pGameConfig, char *psError = NULL, size_t nMaxLength = 0);
			bool LoadEntitySystem(IGameData *pRoot, KeyValues *pGameConfig, char *psError = NULL, size_t nMaxLength = 0);
			bool LoadEntitySpawnGroup(IGameData *pRoot, KeyValues *pGameConfig, char *psError = NULL, size_t nMaxLength = 0);

		public:
			class EntityKeyValues
			{
			public:
				EntityKeyValues();

			public:
				bool Load(IGameData *pRoot, KeyValues *pGameConfig, char *psError = NULL, size_t nMaxLength = 0);
				void Reset();

			public:
				typedef void (*OnEntityKeyValuesPtr)(CEntityKeyValues * const pThis, CUtlScratchMemoryPool *pMemoryPool, char eContainerType);
				typedef CEntityKeyValuesAttribute *(*OnGetAttributePtr)(CEntityKeyValues * const pThis, const EntityKey &key, char *psValue);
				typedef const char *(*OnAttributeGetValueStringPtr)(CEntityKeyValuesAttribute * const pThis, const char *pszDefaultValue);
				typedef void (*OnSetAttributeValuePtr)(CEntityKeyValues * const pThis, CEntityKeyValuesAttribute *pAttribute, const char *pString);

				OnEntityKeyValuesPtr EntityKeyValuesFunction() const;
				OnGetAttributePtr GetAttributeFunction() const;
				OnAttributeGetValueStringPtr AttributeGetValueStringFunction() const;
				OnSetAttributeValuePtr SetAttributeValueFunction() const;

			public:
				ptrdiff_t GetSizeof() const;
				ptrdiff_t GetRefCountOffset() const;

			private:
				GameData::Config::Addresses::ListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config::Offsets::ListenerCallbacksCollector m_aOffsetCallbacks;
				GameData::Config m_aGameConfig;

			private: // Signatures.
				OnEntityKeyValuesPtr m_pfnEntityKeyValues = nullptr;
				OnGetAttributePtr m_pfnGetAttribute = nullptr;
				OnAttributeGetValueStringPtr m_pfnAttributeGetValueString = nullptr;
				OnSetAttributeValuePtr m_pfnSetAttributeValue = nullptr;

			private: // Offsets.
				ptrdiff_t m_nSizeof = -1;
				ptrdiff_t m_nRefCountOffset = -1;
			};

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
				typedef void (*OnExecuteQueuedCreationPtr)(CEntitySystem * const pThis);

				OnCreateEntityPtr CreateEntityFunction() const;
				OnQueueSpawnEntityPtr QueueSpawnEntityFunction() const;
				OnExecuteQueuedCreationPtr ExecuteQueuedCreationFunction() const;

				ptrdiff_t GetGameResourceServiceEntitySystemOffset() const;
				ptrdiff_t GetCurrentManifestOffset() const;
				ptrdiff_t GetKeyValuesMemoryPoolOffset() const;

			private: // Signatures.
				GameData::Config::Addresses::ListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config::Offsets::ListenerCallbacksCollector m_aOffsetCallbacks;
				GameData::Config m_aGameConfig;

			private: // Signatures.
				OnCreateEntityPtr m_pfnCreateEntity = nullptr;
				OnQueueSpawnEntityPtr m_pfnQueueSpawnEntity = nullptr;
				OnExecuteQueuedCreationPtr m_pfnExecuteQueuedCreation = nullptr;

			private: // Offsets.
				ptrdiff_t m_nGameResourceServiceEntitySystemOffset = -1;
				ptrdiff_t m_nCurrentManifestOffset = -1;
				ptrdiff_t m_nKeyValuesMemoryPoolOffset = -1;
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

			private:
				CSpawnGroupMgrGameSystem **m_ppSpawnGroupMgrAddress = nullptr;
				ptrdiff_t m_nMgrGameSystemSpawnGroupsOffset = -1;

			private:
				GameData::Config::Addresses::ListenerCallbacksCollector m_aAddressCallbacks;
				GameData::Config::Offsets::ListenerCallbacksCollector m_aOffsetCallbacks;
				GameData::Config m_aGameConfig;
			};

			const EntityKeyValues &GetEntityKeyValues() const;
			const EntitySystem &GetEntitySystem() const;
			const SpawnGroup &GetSpawnGroup() const;

		private:
			EntityKeyValues m_aEntityKeyValues;
			EntitySystem m_aEntitySystem;
			SpawnGroup m_aSpawnGroup;
		};

		const GameDataStorage &GetGameDataStorage() const;

	private:
		GameData m_aData;
		GameDataStorage m_aStorage;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_H_
