#include "provider_agent.h"

#include "provider/entitysystem.h"

#include <tier0/dbg.h>
#include <tier1/generichash.h>
#include <tier1/KeyValues.h>

extern CGameEntitySystem *g_pGameEntitySystem;

inline EntityKey CalcEntityKey(const char *pszName, const char *pszSafeName, int nLength)
{
	return {MurmurHash2LowerCase(pszName, nLength, ENTITY_KEY_MAGIC_MEOW), pszSafeName};
}

inline EntityKey CalcEntityKey(const char *pszName)
{
	return CalcEntityKey(pszName, pszName, strlen(pszName));
}

class CDefOpsString
{
public:
	static bool LessFunc( const CUtlString &lhs, const CUtlString &rhs )
	{
		return StringLessThan(lhs.Get(), rhs.Get());
	}
};

EntityManagerSpace::ProviderAgent::ProviderAgent()
 :   m_mapCachedKeys(CDefOpsString::LessFunc)
{
	// Cache the classname.
	{
		static const char szClassname[] = "classname";

		const int iClassnameLength = sizeof(szClassname);

		const CUtlString sClassname((const char *)szClassname, iClassnameLength);

		this->m_nElmCachedClassnameKey = this->m_mapCachedKeys.Insert(sClassname, CalcEntityKey((const char *)szClassname, sClassname.Get(), iClassnameLength));
	}
}

bool EntityManagerSpace::ProviderAgent::Init(char *psError, size_t nMaxLength)
{
	return true;
}

void EntityManagerSpace::ProviderAgent::Clear()
{
	this->ReleaseSpawnQueued();
}

void EntityManagerSpace::ProviderAgent::Destroy()
{
}

void EntityManagerSpace::ProviderAgent::PushSpawnQueueOld(KeyValues *pOldKeyValues, SpawnGroupHandle_t hSpawnGroup)
{
	int iNewIndex = this->m_vecEntitySpawnQueue.Count();

	CEntityKeyValuesProvider *pNewKeyValues = (CEntityKeyValuesProvider *)CEntityKeyValuesProvider::Create();

	FOR_EACH_VALUE(pOldKeyValues, pKeyValue)
	{
		const char *pszKey = pKeyValue->GetName();

		CEntityKeyValuesAttribute *pAttr = pNewKeyValues->GetAttribute(this->GetCachedEntityKey(this->CacheOrGetEntityKey(pszKey)));

		if(pAttr)
		{
			const char *pszValue = pKeyValue->GetString(NULL);

			DebugMsg("Queue entity #%d: \"%s\" has \"%s\" value\n", iNewIndex, pszKey, pszValue);

			pNewKeyValues->SetAttributeValue(pAttr, pszValue);
		}
		else
		{
			Warning("Failed to get \"%s\" attribute\n", pszKey);
		}
	}

	this->PushSpawnQueue(pNewKeyValues, hSpawnGroup);
}

void EntityManagerSpace::ProviderAgent::PushSpawnQueue(CEntityKeyValues *pKeyValues, SpawnGroupHandle_t hSpawnGroup)
{
	this->m_vecEntitySpawnQueue.AddToTail({pKeyValues, hSpawnGroup});
}

int EntityManagerSpace::ProviderAgent::AddSpawnQueuedToTail(CUtlVector<const CEntityKeyValues *> *vecTarget)
{
	const auto &vecEntitySpawnQueue = this->m_vecEntitySpawnQueue;

	int iQueueLength = vecEntitySpawnQueue.Count();

	CEntityKeyValues **ppKeyValuesArr = new CEntityKeyValues *[iQueueLength];

	// Fill ppKeyValuesArr from .
	{
		CEntityKeyValues **ppKeyValuesCur = ppKeyValuesArr;

		for(int i = 0; i < iQueueLength; i++)
		{
			*ppKeyValuesCur = vecEntitySpawnQueue[i].GetKeyValues();
			ppKeyValuesCur++;
		}
	}

	int iResult = vecTarget->AddMultipleToTail(iQueueLength, ppKeyValuesArr);

	delete[] ppKeyValuesArr;

	return iResult;
}

int EntityManagerSpace::ProviderAgent::AddSpawnQueuedToTail(CUtlVector<const CEntityKeyValues *> *vecTarget, SpawnGroupHandle_t hSpawnGroup)
{
	const auto &vecEntitySpawnQueue = this->m_vecEntitySpawnQueue;

	int iQueueLength = vecEntitySpawnQueue.Count();

	CEntityKeyValues **ppKeyValuesArr = new CEntityKeyValues *[iQueueLength];

	// Fill ppKeyValuesArr from .
	{
		CEntityKeyValues **ppKeyValuesCur = ppKeyValuesArr;

		for(int i = 0; i < iQueueLength; i++)
		{
			const EntityData &aSpawnEntity = vecEntitySpawnQueue[i];

			if(hSpawnGroup == aSpawnEntity.GetSpawnGroup())
			{
				*ppKeyValuesCur = aSpawnEntity.GetKeyValues();
				ppKeyValuesCur++;
			}
		}
	}

	int iResult = vecTarget->AddMultipleToTail(iQueueLength, ppKeyValuesArr);

	delete[] ppKeyValuesArr;

	return iResult;
}

void EntityManagerSpace::ProviderAgent::ReleaseSpawnQueued()
{
	this->m_vecEntitySpawnQueue.Purge();
}

int EntityManagerSpace::ProviderAgent::ReleaseSpawnQueued(SpawnGroupHandle_t hSpawnGroup)
{
	auto &vecEntitySpawnQueue = this->m_vecEntitySpawnQueue;

	const int iQueueLengthBefore = vecEntitySpawnQueue.Count();

	// Once find and fast remove.
	for(int i = 0; i < vecEntitySpawnQueue.Count(); i++)
	{
		if(hSpawnGroup == vecEntitySpawnQueue[i].GetSpawnGroup())
		{
			vecEntitySpawnQueue.FastRemove(i);
			i--;
		}
	}

	return iQueueLengthBefore - vecEntitySpawnQueue.Count();
}

int EntityManagerSpace::ProviderAgent::SpawnQueued()
{
	CEntitySystemProvider *pEntitySystem = (CEntitySystemProvider *)g_pGameEntitySystem;

	auto &vecEntitySpawnQueue = this->m_vecEntitySpawnQueue;

	char psClassname[128] = "\0", 
	     sEntityError[256];

	const int iQueueLength = vecEntitySpawnQueue.Count();

	CUtlVector<CUtlString> vecErrors;

	const CEntityIndex iForceEdictIndex = CEntityIndex(-1);

	const auto &aClassnameKey = this->GetCachedClassnameKey();

	for(int i = 0; i < iQueueLength; i++)
	{
		const EntityData &aSpawnEntity = vecEntitySpawnQueue[i];

		CEntityKeyValuesProvider *pKeyValues = aSpawnEntity.GetKeyValuesProvider();

		CEntityKeyValuesAttributeProvider *pAttr = (CEntityKeyValuesAttributeProvider *)pKeyValues->GetAttribute(aClassnameKey);

		if(pAttr)
		{
			const char *pszClassname = pAttr->GetValueString();

			if(pszClassname && pszClassname[0])
			{
				CBaseEntity *pEntity = (CBaseEntity *)pEntitySystem->CreateEntity(aSpawnEntity.GetSpawnGroup(), pszClassname, ENTITY_NETWORKING_MODE_DEFAULT, iForceEdictIndex, -1, false);

				if(pEntity)
				{
					DebugMsg("Created \"%s\" (force edict index is %d, result index is %d) entity\n", pszClassname, iForceEdictIndex.Get(), pEntity->m_pEntity->m_EHandle.GetEntryIndex());

					pEntitySystem->QueueSpawnEntity(pEntity->m_pEntity, pKeyValues);
				}
				else
				{
					snprintf((char *)sEntityError, sizeof(sEntityError), "Failed to create \"%s\" entity (queue number is %d)", pszClassname, i);
					vecErrors.AddToTail((const char *)sEntityError);
				}
			}
			else
			{
				snprintf((char *)sEntityError, sizeof(sEntityError), "Empty entity \"%s\" key (queue number is %d)", aClassnameKey.m_pszName, i);
				vecErrors.AddToTail((const char *)sEntityError);
			}
		}
		else
		{
			snprintf((char *)sEntityError, sizeof(sEntityError), "Failed to get \"%s\" entity key (queue number is %d)", aClassnameKey.m_pszName, i);
			vecErrors.AddToTail((const char *)sEntityError);
		}
	}

	// Print an errors.
	FOR_EACH_VEC(vecErrors, i)
	{
		Warning("%s\n", vecErrors[i].Get());
	}

	if(vecErrors.Count() < this->m_vecEntitySpawnQueue.Count())
	{
		pEntitySystem->ExecuteQueuedCreation();
	}

	this->ReleaseSpawnQueued();

	return iQueueLength;
}

int EntityManagerSpace::ProviderAgent::SpawnQueued(SpawnGroupHandle_t hSpawnGroup)
{
	CEntitySystemProvider *pEntitySystem = (CEntitySystemProvider *)g_pGameEntitySystem;

	auto &vecEntitySpawnQueue = this->m_vecEntitySpawnQueue;

	char sEntityError[256];

	const int iQueueLength = vecEntitySpawnQueue.Count();

	CUtlVector<CUtlString> vecErrors;

	const CEntityIndex iForceEdictIndex = CEntityIndex(-1);

	const auto &aClassnameKey = this->GetCachedClassnameKey();

	for(int i = 0; i < vecEntitySpawnQueue.Count(); i++)
	{
		const EntityData &aSpawnEntity = vecEntitySpawnQueue[i];

		if(hSpawnGroup == aSpawnEntity.GetSpawnGroup())
		{
			CEntityKeyValuesProvider *pKeyValues = aSpawnEntity.GetKeyValuesProvider();

			CEntityKeyValuesAttributeProvider *pAttr = (CEntityKeyValuesAttributeProvider *)pKeyValues->GetAttribute(aClassnameKey);

			if(pAttr)
			{
				const char *pszClassname = pAttr->GetValueString();

				if(pszClassname && pszClassname[0])
				{
					CBaseEntity *pEntity = (CBaseEntity *)pEntitySystem->CreateEntity(hSpawnGroup, pszClassname, ENTITY_NETWORKING_MODE_DEFAULT, iForceEdictIndex, -1, false);

					if(pEntity)
					{
						DebugMsg("Created \"%s\" (force edict index is %d, result index is %d) entity\n", pszClassname, iForceEdictIndex.Get(), pEntity->m_pEntity->m_EHandle.GetEntryIndex());

						pEntitySystem->QueueSpawnEntity(pEntity->m_pEntity, pKeyValues);
					}
					else
					{
						snprintf((char *)sEntityError, sizeof(sEntityError), "Failed to create \"%s\" entity (queue number is %d)", pszClassname, i);
						vecErrors.AddToTail((const char *)sEntityError);
					}
				}
				else
				{
					snprintf((char *)sEntityError, sizeof(sEntityError), "Empty entity \"%s\" key (queue number is %d)", aClassnameKey.m_pszName, i);
					vecErrors.AddToTail((const char *)sEntityError);
				}
			}
			else
			{
				snprintf((char *)sEntityError, sizeof(sEntityError), "Failed to get \"%s\" entity key (queue number is %d)", aClassnameKey.m_pszName, i);
				vecErrors.AddToTail((const char *)sEntityError);
			}

			vecEntitySpawnQueue.FastRemove(i);
			i--;
		}
	}

	// Print an errors.
	FOR_EACH_VEC(vecErrors, i)
	{
		Warning("%s\n", vecErrors[i].Get());
	}

	const int iCompliteQueueLength = this->m_vecEntitySpawnQueue.Count();

	if(vecErrors.Count() < iCompliteQueueLength)
	{
		pEntitySystem->ExecuteQueuedCreation();
	}

	return iQueueLength - iCompliteQueueLength;
}

const EntityKey &EntityManagerSpace::ProviderAgent::GetCachedEntityKey(CacheMapOIndexType nElm)
{
	return this->m_mapCachedKeys[nElm];
}

const EntityKey &EntityManagerSpace::ProviderAgent::GetCachedClassnameKey()
{
	return this->m_mapCachedKeys[this->m_nElmCachedClassnameKey];
}

EntityManagerSpace::ProviderAgent::CacheMapOIndexType EntityManagerSpace::ProviderAgent::CacheOrGetEntityKey(const char *pszName)
{
	const CUtlString sName = pszName;

	CacheMapOIndexType nFindElm = this->m_mapCachedKeys.Find(sName);

	return nFindElm == this->m_mapCachedKeys.InvalidIndex() ? this->m_mapCachedKeys.Insert(sName, CalcEntityKey(pszName, sName.Get(), sName.Length())) : nFindElm;
}

EntityManagerSpace::ProviderAgent::EntityData::EntityData(CEntityKeyValues *pKeyValues)
{
	this->m_pKeyValues = pKeyValues;
	this->m_hSpawnGroup = ThisClass::GetAnySpawnGroup();
}

EntityManagerSpace::ProviderAgent::EntityData::EntityData(CEntityKeyValues *pKeyValues, SpawnGroupHandle_t hSpawnGroup)
{
	this->m_pKeyValues = pKeyValues;
	this->m_hSpawnGroup = hSpawnGroup;
}

CEntityKeyValues *EntityManagerSpace::ProviderAgent::EntityData::GetKeyValues() const
{
	return this->m_pKeyValues;
}

EntityManagerSpace::CEntityKeyValuesProvider *EntityManagerSpace::ProviderAgent::EntityData::GetKeyValuesProvider() const
{
	return (CEntityKeyValuesProvider *)this->m_pKeyValues;
}

SpawnGroupHandle_t EntityManagerSpace::ProviderAgent::EntityData::GetSpawnGroup() const
{
	return this->m_hSpawnGroup;
}

SpawnGroupHandle_t EntityManagerSpace::ProviderAgent::EntityData::GetAnySpawnGroup()
{
	return (SpawnGroupHandle_t)-1;
}

bool EntityManagerSpace::ProviderAgent::EntityData::IsAnySpawnGroup() const
{
	return this->GetSpawnGroup() == ThisClass::GetAnySpawnGroup();
}
