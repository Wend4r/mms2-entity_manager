#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_H_

#include <stddef.h>
#include <stdint.h>

#include <tier0/platform.h>

#include "provider/entitykeyvalues.h"
#include "provider/entitysystem.h"

namespace EntityManager
{
	class Provider
	{
	public:
		friend class CEntityKeyValuesProvider;
		friend class CEntityKeyValuesAttributeProvider;
		friend class CEntitySystemProvider;
		friend class CSpawnGroupMgrGameSystemProvider;

		bool Init(char *psError, size_t nMaxLength);
		bool Load(char *psError = NULL, size_t nMaxLength = 0);
		void Destroy();

	protected:
		bool LoadGameData(char *psError = NULL, size_t nMaxLength = 0);
		bool LoadEntityKeyValuesGameData(char *psError = NULL, size_t nMaxLength = 0);
		bool LoadEntitySystemGameData(char *psError = NULL, size_t nMaxLength = 0);
		bool LoadSpawnGroupGameData(char *psError = NULL, size_t nMaxLength = 0);

	private:
		class GameData
		{
		public:
			class EntityKeyValues
			{
			public:
				typedef void (*EntityKeyValuesFuncType)(CEntityKeyValues * const pThis, void *pEntitySystemSubobject, char eSubobjectType);
				EntityKeyValuesFuncType m_pfnEntityKeyValues = nullptr;

				typedef CEntityKeyValuesAttribute *(*GetAttributeFuncType)(CEntityKeyValues * const pThis, const EntityKey &key, char *psValue);
				GetAttributeFuncType m_pfnGetAttribute = nullptr;

				typedef const char *(*GetValueStringFuncType)(CEntityKeyValuesAttribute * const pThis, const char *pszDefaultValue);
				GetValueStringFuncType m_pfnGetValueString = nullptr;

				typedef void (*SetAttributeValueFuncType)(CEntityKeyValues * const pThis, CEntityKeyValuesAttribute *pAttribute, const char *pString);
				SetAttributeValueFuncType m_pfnSetAttributeValue = nullptr;

				ptrdiff_t m_nSizeof = -1;

				ptrdiff_t m_nRefCountOffset = -1;
			};

			EntityKeyValues m_aEntityKeyValues;

			class EntitySystem
			{
			public:
				typedef CEntityInstance *(*CreateEntityFuncType)(CEntitySystem * const pThis, SpawnGroupHandle_t hSpawnGroup, const char *pszNameOrDesignName, EntityNetworkingMode_t eNetworkMode, CEntityIndex iForcedIndex, int iForcedSerial, bool bCreateInIsolatedPrecacheList);
				CreateEntityFuncType m_pfnCreateEntity = nullptr;

				typedef void (*QueueSpawnEntityFuncType)(CEntitySystem * const pThis, CEntityIdentity *pEntity, const CEntityKeyValues *pInitializationData);
				QueueSpawnEntityFuncType m_pfnQueueSpawnEntity = nullptr;

				typedef void (*ExecuteQueuedCreationFuncType)(CEntitySystem * const pThis);
				ExecuteQueuedCreationFuncType m_pfnExecuteQueuedCreation = nullptr;

				ptrdiff_t m_nSubobjectForKeyValuesOffset = -1;
			};

			EntitySystem m_aEntitySystem;

			class SpawnGroup
			{
			public:
				ptrdiff_t m_nMgrGameSystemSpawnGroupsOffset = -1;
			};

			SpawnGroup m_aSpawnGroup;
		};

		GameData m_aData;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_H_
