#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_

#include <stddef.h>

#include <eiface.h>
#include <tier1/utlmap.h>
#include <tier1/utlstring.h>
#include <tier1/utlvector.h>
#include <entity2/entityidentity.h>

#include "provider/entitykeyvalues.h"

class KeyValues;
class CEntityKeyValues;

namespace EntityManager
{
	class ProviderAgent
	{
	public:
		ProviderAgent();

		bool Init(char *psError, size_t nMaxLength);
		void Clear();
		void Destroy();

	public:
		bool NotifyEntitySystemUpdated();
		bool NotifySpawnGroupMgrUpdated();

	public: // Spawn queue methods.
		void PushSpawnQueueOld(KeyValues *pOldKeyValues, SpawnGroupHandle_t hSpawnGroup = (SpawnGroupHandle_t)-1);
		void PushSpawnQueue(CEntityKeyValues *pKeyValues, SpawnGroupHandle_t hSpawnGroup = (SpawnGroupHandle_t)-1);
		int AddSpawnQueueToTail(CUtlVector<const CEntityKeyValues *> *vecTarget);
		int AddSpawnQueueToTail(CUtlVector<const CEntityKeyValues *> *vecTarget, SpawnGroupHandle_t hSpawnGroup);
		void ReleaseSpawnQueued();
		int ReleaseSpawnQueued(SpawnGroupHandle_t hSpawnGroup);
		int SpawnQueued();
		int SpawnQueued(SpawnGroupHandle_t hSpawnGroup);

	protected:
		typedef CUtlMap<CUtlString, EntityKey> CacheMapType;
		typedef CacheMapType::IndexType_t CacheMapOIndexType;

		const EntityKey &GetCachedEntityKey(CacheMapOIndexType nElm);
		const EntityKey &GetCachedClassnameKey();

		CacheMapOIndexType CacheOrGetEntityKey(const char *pszName);

	private:
		class EntityData
		{
			typedef EntityData ThisClass;

		public:
			EntityData(CEntityKeyValues *pKeyValues);
			EntityData(CEntityKeyValues *pKeyValues, SpawnGroupHandle_t hSpawnGroup);

			CEntityKeyValues *GetKeyValues() const;
			CEntityKeyValuesProvider *GetKeyValuesProvider() const;

			SpawnGroupHandle_t GetSpawnGroup() const;
			static SpawnGroupHandle_t GetAnySpawnGroup();
			bool IsAnySpawnGroup() const;

		private:
			CEntityKeyValues *m_pKeyValues;
			SpawnGroupHandle_t m_hSpawnGroup;
		};

		CUtlVector<EntityData> m_vecEntitySpawnQueue; // To spawn.

		CacheMapType m_mapCachedKeys;
		CacheMapOIndexType m_nElmCachedClassnameKey;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_
