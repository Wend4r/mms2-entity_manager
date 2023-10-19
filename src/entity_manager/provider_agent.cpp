#include "provider_agent.h"

#include "provider/entitykeyvalues.h"
#include "provider/entitysystem.h"

#include <tier1/generichash.h>
#include <tier1/KeyValues.h>

extern CGameEntitySystem *g_pGameEntitySystem;

inline EntityKey CalcEntityKey(const char *pszName, int nLength)
{
	return {MurmurHash2LowerCase(pszName, nLength, ENTITY_KEY_MAGIC_MEOW), pszName};
}

inline EntityKey CalcEntityKey(const char *pszName)
{
	return CalcEntityKey(pszName, strlen(pszName));
}

bool EntityManagerSpace::ProviderAgent::Init(char *psError, size_t nMaxLength)
{
	return true;
}

void EntityManagerSpace::ProviderAgent::Clear()
{
	this->m_vecEntitySpawnQueue.Purge();
}

void EntityManagerSpace::ProviderAgent::Destroy()
{
}

void EntityManagerSpace::ProviderAgent::PushSpawnQueue(KeyValues *pEntityValues)
{
	int iNewIndex = this->m_vecEntitySpawnQueue.Count();

	CEntityKeyValuesProvider *pNewKeyValues = (CEntityKeyValuesProvider *)CEntityKeyValuesProvider::Create();

	FOR_EACH_VALUE(pEntityValues, pKeyValue)
	{
		const char *pszKey = pKeyValue->GetName();

		void *pAttr = pNewKeyValues->GetAttribute(CalcEntityKey(pszKey));

		if(pAttr)
		{
			const char *pszValue = pKeyValue->GetString(NULL);

			DebugMsg("Queue entity #%d: \"%s\" has \"%s\" value\n", iNewIndex, pszKey, pszValue);

			pNewKeyValues->SetAttributeValue(pAttr, pszValue);
		}
		else
		{
			Warning("Failed to get \"%s\" attribute", pszKey);
		}
	}

	this->m_vecEntitySpawnQueue.AddToTail({pNewKeyValues});
}

int EntityManagerSpace::ProviderAgent::SpawnQueued()
{
	CEntitySystemProvider *pEntitySystem = (CEntitySystemProvider *)g_pGameEntitySystem;

	int iQueueLength = this->m_vecEntitySpawnQueue.Count();

	{

		char psClassname[128] = "\0", 
		     sEntityError[256];

		static const char szClassnameKey[] = "classname";

		CUtlVector<CUtlString> vecErrors;

		const CEntityIndex iForceEdictIndex = CEntityIndex(-1);

		const EntityKey aClassnameKey = CalcEntityKey(szClassnameKey, sizeof(szClassnameKey));

		for(int i = 0; i < iQueueLength; i++)
		{
			CEntityKeyValuesProvider *pKeyValues = (CEntityKeyValuesProvider *)this->m_vecEntitySpawnQueue[i].m_pKeyValues;

			if(pKeyValues->GetAttribute(aClassnameKey, (char *)psClassname) && psClassname[0])
			{
				CBaseEntity *pEntity = (CBaseEntity *)pEntitySystem->CreateEntity(iForceEdictIndex, (const char *)psClassname, ENTITY_NETWORKING_MODE_DEFAULT, (SpawnGroupHandle_t)-1, -1, false);

				if(pEntity)
				{
					int iIndex = pEntity->m_pEntity->m_EHandle.GetEntryIndex();

					DebugMsg("Created \"%s\" (force edict index is %d, result index is %d) entity\n", psClassname, iForceEdictIndex.Get(), iIndex);

					pEntitySystem->QueueSpawnEntity(pEntity->m_pEntity, pKeyValues);
				}
				else
				{
					snprintf((char *)sEntityError, sizeof(sEntityError), "Failed to create \"%s\" entity (queue number is %d)", psClassname, i);
					vecErrors.AddToTail((const char *)sEntityError);
				}
			}
			else
			{
				snprintf((char *)sEntityError, sizeof(sEntityError), "Failed to get \"%s\" key of entity queue #%d", aClassnameKey.m_pszName, i);
				vecErrors.AddToTail((const char *)sEntityError);
			}
		}

		// Print an errors.
		FOR_EACH_VEC(vecErrors, i)
		{
			Warning("Failed to create entity: %s", vecErrors[i].Get());
		}

		if(vecErrors.Count() < this->m_vecEntitySpawnQueue.Count())
		{
			pEntitySystem->ExecuteQueuedCreation();
		}

		this->m_vecEntitySpawnQueue.Purge();
	}

	return iQueueLength;
}
