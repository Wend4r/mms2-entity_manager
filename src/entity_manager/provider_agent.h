#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_AGENT_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_AGENT_

#include <stddef.h>

#include <tier1/utlvector.h>

class KeyValues;
class CEntityKeyValues;

namespace EntityManagerSpace
{
	class ProviderAgent
	{
	public:
		bool Init(char *psError, size_t nMaxLength);
		void Clear();
		void Destroy();

	public:
		void PushSpawnQueue(KeyValues *pEntityValues);
		int SpawnQueued();

	private:
		struct EntityData
		{
			CEntityKeyValues *m_pKeyValues;
		};

		CUtlVector<EntityData> m_vecEntitySpawnQueue; // To spawn.
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_AGENT_
