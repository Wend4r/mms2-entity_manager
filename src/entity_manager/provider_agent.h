#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_AGENT_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_AGENT_

#include <stddef.h>

#include <eiface.h>
#include <tier1/utlmap.h>
#include <tier1/utlstring.h>
#include <tier1/utlvector.h>
#include <entity2/entityidentity.h>

#include "provider/entitykeyvalues.h"

class KeyValues;
class CEntityKeyValues;

namespace EntityManagerSpace
{
	class ProviderAgent
	{
	public:
		ProviderAgent();

		bool Init(char *psError, size_t nMaxLength);
		void Clear();
		void Destroy();

	public:
		void PushSpawnQueue(KeyValues *pEntityValues);
		int SpawnQueued(SpawnGroupHandle_t hSpawnGroup);

	protected:
		typedef CUtlMap<CUtlString, EntityKey> CacheMapType;
		typedef CacheMapType::IndexType_t CacheMapOIndexType;

		const EntityKey &GetCachedEntityKey(CacheMapOIndexType nElm);
		const EntityKey &GetCachedClassnameKey();

		CacheMapOIndexType CacheOrGetEntityKey(const char *pszName);

	private:
		struct EntityData
		{
			CEntityKeyValues *m_pKeyValues;
		};

		CUtlVector<EntityData> m_vecEntitySpawnQueue; // To spawn.

		CacheMapType m_mapCachedKeys;
		CacheMapOIndexType m_nElmCachedClassnameKey;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_AGENT_
