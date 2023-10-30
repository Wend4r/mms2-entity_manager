#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_

#include <stddef.h>

#include <eiface.h>
#include <tier1/utlmap.h>
#include <tier1/utlstring.h>
#include <tier1/utlvector.h>
#include <entity2/entityidentity.h>

#include "logger.h"
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
		bool HasInSpawnQueue(const CEntityKeyValues *pKeyValues);
		void ReleaseSpawnQueued();
		int ReleaseSpawnQueued(SpawnGroupHandle_t hSpawnGroup);
		int SpawnQueued();
		int SpawnQueued(SpawnGroupHandle_t hSpawnGroup);

	public: // Destroy queue methods.
		void PushDestroyQueue(CEntityInstance *pEntity);
		void PushDestroyQueue(CEntityIdentity *pEntity);
		int AddDestroyQueueToTail(CUtlVector<const CEntityIdentity *> *vecTarget);
		void ReleaseDestroyQueued();
		int DestroyQueued();

	public:
		void DumpEntityKeyValues(Logger::Scope &aOutput, const CEntityKeyValues *pKeyValues);

	protected:
		typedef CUtlMap<CUtlString, EntityKey> CacheMapType;
		typedef CacheMapType::IndexType_t CacheMapOIndexType;

		const EntityKey &GetCachedEntityKey(CacheMapOIndexType nElm);
		const EntityKey &GetCachedClassnameKey();

		CacheMapOIndexType CacheOrGetEntityKey(const char *pszName);

	private:
		class SpawnData
		{
			typedef SpawnData ThisClass;

		public:
			SpawnData(CEntityKeyValues *pKeyValues);
			SpawnData(CEntityKeyValues *pKeyValues, SpawnGroupHandle_t hSpawnGroup);
			~SpawnData();

			CEntityKeyValues *GetKeyValues() const;
			CEntityKeyValuesProvider *GetKeyValuesProvider() const;

			SpawnGroupHandle_t GetSpawnGroup() const;
			static SpawnGroupHandle_t GetAnySpawnGroup();
			bool IsAnySpawnGroup() const;

			void Release();

		private:
			CEntityKeyValues *m_pKeyValues;
			SpawnGroupHandle_t m_hSpawnGroup;
		};

		CUtlVector<SpawnData> m_vecEntitySpawnQueue;

		struct DestoryData
		{
			typedef DestoryData ThisClass;

		public:
			DestoryData(CEntityInstance *pInstance);
			DestoryData(CEntityIdentity *pIdentity);
			~DestoryData();

			CEntityIdentity *GetIdnetity() const;

		private:
			CEntityIdentity *m_pIdentity;
		};

		CUtlVector<DestoryData> m_vecEntityDestroyQueue;

		CacheMapType m_mapCachedKeys;
		CacheMapOIndexType m_nElmCachedClassnameKey;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_AGENT_
