#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_H_

#include <stddef.h>
#include <stdint.h>

#include <tier0/platform.h>

#include "provider/entitykeyvalues.h"
#include "provider/entitysystem.h"

namespace EntityManagerSpace
{
	class Provider
	{
	public:
		friend class CEntityKeyValuesProvider;
		friend class CEntitySystemProvider;

		bool Init(char *psError, size_t nMaxLength);
		bool Load(char *psError = NULL, size_t nMaxLength = 0);
		void Destroy();

	protected:
		bool LoadGameData(char *psError = NULL, size_t nMaxLength = 0);
		bool LoadEntityKeyValuesGameData(char *psError = NULL, size_t nMaxLength = 0);
		bool LoadEntitySystemGameData(char *psError = NULL, size_t nMaxLength = 0);

	private:
		class GameData
		{
		public:
			class EntityKeyValues
			{
			public:
				typedef void (*EntityKeyValuesFuncType)(CEntityKeyValues * const pThis, void *pEntitySystemSubobject, char eSubobjectType);
				EntityKeyValuesFuncType m_pfnEntityKeyValues;

				typedef void *(*GetAttributeFuncType)(CEntityKeyValues * const pThis, const EntityKey &key, char *psValue);
				GetAttributeFuncType m_pfnGetAttribute;

				typedef void (*SetAttributeValueFuncType)(CEntityKeyValues * const pThis, void *pAttribute, const char *pString);
				SetAttributeValueFuncType m_pfnSetAttributeValue;

				size_t m_nSizeof;
			};

			EntityKeyValues m_aEntityKeyValues;

			class EntitySystem
			{
			public:
				typedef CEntityInstance *(*CreateEntityFuncType)(CEntitySystem * const pThis, CEntityIndex iForcedIndex, const char *pszNameOrDesignName, EntityNetworkingMode_t eNetworkMode, SpawnGroupHandle_t hSpawnGroup, int iForcedSerial, bool bCreateInIsolatedPrecacheList);
				CreateEntityFuncType m_pfnCreateEntity;

				typedef void (*QueueSpawnEntityFuncType)(CEntitySystem * const pThis, CEntityIdentity *pEntity, const CEntityKeyValues *pInitializationData);
				QueueSpawnEntityFuncType m_pfnQueueSpawnEntity;

				typedef void (*ExecuteQueuedCreationFuncType)(CEntitySystem * const pThis);
				ExecuteQueuedCreationFuncType m_pfnExecuteQueuedCreation;
			};

			EntitySystem m_aEntitySystem;
		};

		GameData m_aData;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_H_
