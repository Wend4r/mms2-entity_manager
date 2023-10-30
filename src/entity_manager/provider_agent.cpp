#include "provider_agent.h"

#include "provider.h"
#include "provider/entitysystem.h"
#include "logger.h"

#include <tier0/dbg.h>
#include <tier0/platform.h>
#include <tier1/generichash.h>
#include <tier1/KeyValues.h>

extern EntityManager::Logger *g_pEntityManagerLogger;
extern EntityManager::Provider *g_pEntityManagerProvider;

extern IGameResourceServiceServer *g_pGameResourceServiceServer;
CGameEntitySystem *g_pGameEntitySystem = NULL;
CSpawnGroupMgrGameSystem *g_pSpawnGroupMgr = NULL;

// tier0 module.
PLATFORM_INTERFACE bool g_bUpdateStringTokenDatabase;
PLATFORM_INTERFACE void RegisterStringToken(uint32 nHashCode, const char *pszName, uint64 nUnk1 = 0LL, bool bUnk2 = true);

// Entity key caclulation.
FORCEINLINE const EntityKey CalcEntityKey(const char *pszName, const char *pszSafeName, int nLength)
{
	return {MurmurHash2LowerCase(pszName, nLength, ENTITY_KEY_MAGIC_MEOW), pszSafeName};
}

FORCEINLINE const EntityKey CalcEntityKey(const char *pszName, int nLength)
{
	return CalcEntityKey(pszName, pszName, nLength);
}

FORCEINLINE const EntityKey CalcEntityKey(const char *pszName)
{
	return CalcEntityKey(pszName, pszName, strlen(pszName));
}

// Make an entity key.
FORCEINLINE const EntityKey MakeEntityKey(const char *pszName, const char *pszSafeName, int nLength)
{
	const EntityKey aKey = CalcEntityKey(pszName, pszSafeName, nLength);

	if(g_bUpdateStringTokenDatabase)
	{
		RegisterStringToken(aKey.m_nHashCode, aKey.m_pszName);
	}

	return aKey;
}

FORCEINLINE const EntityKey MakeEntityKey(const char *pszName, int nLength)
{
	return MakeEntityKey(pszName, pszName, nLength);
}

FORCEINLINE const EntityKey MakeEntityKey(const char *pszName)
{
	return MakeEntityKey(pszName, pszName, strlen(pszName));
}

class CDefOpsString
{
public:
	static bool LessFunc( const CUtlString &lhs, const CUtlString &rhs )
	{
		return StringLessThan(lhs.Get(), rhs.Get());
	}
};

EntityManager::ProviderAgent::ProviderAgent()
 :   m_mapCachedKeys(CDefOpsString::LessFunc)
{
	// Cache the classname.
	{
		static const char szClassname[] = "classname";

		this->m_nElmCachedClassnameKey = this->m_mapCachedKeys.Insert((const char *)szClassname, MakeEntityKey((const char *)szClassname, sizeof(szClassname) - 1));
	}
}

bool EntityManager::ProviderAgent::Init(char *psError, size_t nMaxLength)
{
	return true;
}

void EntityManager::ProviderAgent::Clear()
{
	this->ReleaseSpawnQueued();
	this->ReleaseDestroyQueued();
}

void EntityManager::ProviderAgent::Destroy()
{
	this->Clear();
}

bool EntityManager::ProviderAgent::NotifyEntitySystemUpdated()
{
	return (g_pGameEntitySystem = *(CGameEntitySystem **)((uintptr_t)g_pGameResourceServiceServer + g_pEntityManagerProvider->GetGameDataStorage().GetGameResource().GetEntitySystemOffset())) != NULL;
}

bool EntityManager::ProviderAgent::NotifySpawnGroupMgrUpdated()
{
	return (g_pSpawnGroupMgr = *g_pEntityManagerProvider->GetGameDataStorage().GetSpawnGroup().GetSpawnGroupMgrAddress()) != NULL;
}

void EntityManager::ProviderAgent::PushSpawnQueueOld(KeyValues *pOldKeyValues, SpawnGroupHandle_t hSpawnGroup)
{
	int iNewIndex = this->m_vecEntitySpawnQueue.Count();

	CEntityKeyValuesProvider *pNewKeyValues = (CEntityKeyValuesProvider *)CEntityKeyValuesProvider::Create(((CEntitySystemProvider *)g_pGameEntitySystem)->GetKeyValuesMemoryPool(), 3);

	auto aDetails = g_pEntityManagerLogger->CreateDetailsScope(), 
	     aWarnings = g_pEntityManagerLogger->CreateWarningsScope();

	aDetails.PushFormat("- Queue entity #%d -", iNewIndex);

	FOR_EACH_VALUE(pOldKeyValues, pKeyValue)
	{
		const char *pszKey = pKeyValue->GetName();

		CEntityKeyValuesAttribute *pAttr = pNewKeyValues->GetAttribute(this->GetCachedEntityKey(this->CacheOrGetEntityKey(pszKey)));

		if(pAttr)
		{
			const char *pszValue = pKeyValue->GetString(NULL);

			aDetails.PushFormat("  \"%s\" has \"%s\" value", pszKey, pszValue);

			pNewKeyValues->SetAttributeValue(pAttr, pszValue);
		}
		else
		{
			aWarnings.PushFormat("Failed to get \"%s\" attribute", pszKey);
		}
	}

	Color aMsgColor = pOldKeyValues->GetColor("color", {200, 200, 200, 255});

	aDetails.Send([&aMsgColor](const char *pszContent) {
		g_pEntityManagerLogger->Detailed(aMsgColor, pszContent);
	});

	aWarnings.Send([&aMsgColor](const char *pszContent) {
		g_pEntityManagerLogger->Warning(aMsgColor, pszContent);
	});

	this->PushSpawnQueue(pNewKeyValues, hSpawnGroup);
}

void EntityManager::ProviderAgent::PushSpawnQueue(CEntityKeyValues *pKeyValues, SpawnGroupHandle_t hSpawnGroup)
{
	((CEntityKeyValuesProvider *)pKeyValues)->AddRef();

	this->m_vecEntitySpawnQueue.AddToTail({pKeyValues, hSpawnGroup});
}

int EntityManager::ProviderAgent::AddSpawnQueueToTail(CUtlVector<const CEntityKeyValues *> *vecTarget)
{
	const auto &vecEntitySpawnQueue = this->m_vecEntitySpawnQueue;

	int iQueueLength = vecEntitySpawnQueue.Count();

	CEntityKeyValues **ppKeyValuesArr = (CEntityKeyValues **)malloc(iQueueLength * sizeof(CEntityKeyValues *));

	// Fill ppKeyValuesArr from .
	{
		CEntityKeyValues **ppKeyValuesCur = ppKeyValuesArr;

		for(int i = 0; i < iQueueLength; i++)
		{
			CEntityKeyValues *pKeyValues = vecEntitySpawnQueue[i].GetKeyValues();

			((CEntityKeyValuesProvider *)pKeyValues)->AddRef();

			*ppKeyValuesCur = pKeyValues;
			ppKeyValuesCur++;
		}
	}

	int iResult = vecTarget->AddMultipleToTail(iQueueLength, ppKeyValuesArr);

	free(ppKeyValuesArr);

	return iResult;
}

int EntityManager::ProviderAgent::AddSpawnQueueToTail(CUtlVector<const CEntityKeyValues *> *vecTarget, SpawnGroupHandle_t hSpawnGroup)
{
	const auto &vecEntitySpawnQueue = this->m_vecEntitySpawnQueue;

	const int iQueueLength = vecEntitySpawnQueue.Count();

	CEntityKeyValues **ppKeyValuesArr = (CEntityKeyValues **)malloc(iQueueLength * sizeof(CEntityKeyValues *));
	CEntityKeyValues **ppKeyValuesCur = ppKeyValuesArr;

	for(int i = 0; i < iQueueLength; i++)
	{
		const auto &aSpawn = vecEntitySpawnQueue[i];

		if(hSpawnGroup == aSpawn.GetSpawnGroup())
		{
			CEntityKeyValues *pKeyValues = aSpawn.GetKeyValues();

			aSpawn.GetKeyValuesProvider()->AddRef();

			*ppKeyValuesCur = pKeyValues;
			ppKeyValuesCur++;
		}
	}

	int iResult = vecTarget->AddMultipleToTail(((uintptr_t)ppKeyValuesCur - (uintptr_t)ppKeyValuesArr) / sizeof(decltype(ppKeyValuesArr)), ppKeyValuesArr);

	free(ppKeyValuesArr);

	return iResult;
}

bool EntityManager::ProviderAgent::HasInSpawnQueue(const CEntityKeyValues *pKeyValues)
{
	FOR_EACH_VEC(this->m_vecEntitySpawnQueue, i)
	{
		const auto &aSpawn = this->m_vecEntitySpawnQueue[i];

		if(aSpawn.GetKeyValues() == pKeyValues)
		{
			return true;
		}
	}

	return false;
}

void EntityManager::ProviderAgent::ReleaseSpawnQueued()
{
	this->m_vecEntitySpawnQueue.Purge();
}

int EntityManager::ProviderAgent::ReleaseSpawnQueued(SpawnGroupHandle_t hSpawnGroup)
{
	auto &vecEntitySpawnQueue = this->m_vecEntitySpawnQueue;

	const int iQueueLengthBefore = vecEntitySpawnQueue.Count();

	// Find and fast remove.
	for(int i = 0; i < vecEntitySpawnQueue.Count(); i++)
	{
		auto &aSpawn = vecEntitySpawnQueue[i];

		if(hSpawnGroup == aSpawn.GetSpawnGroup())
		{
			vecEntitySpawnQueue.FastRemove(i);
			i--;
		}
	}

	return iQueueLengthBefore - vecEntitySpawnQueue.Count();
}

int EntityManager::ProviderAgent::SpawnQueued()
{
	CEntitySystemProvider *pEntitySystem = (CEntitySystemProvider *)g_pGameEntitySystem;

	auto &vecEntitySpawnQueue = this->m_vecEntitySpawnQueue;

	char psClassname[128] = "\0", 
	     sEntityError[256];

	const int iQueueLength = vecEntitySpawnQueue.Count();

	auto aDetails = g_pEntityManagerLogger->CreateMessagesScope(), 
	     aWarnings = g_pEntityManagerLogger->CreateWarningsScope();

	const CEntityIndex iForceEdictIndex = CEntityIndex(-1);

	const auto &aClassnameKey = this->GetCachedClassnameKey();

	for(int i = 0; i < iQueueLength; i++)
	{
		const auto &aSpawn = vecEntitySpawnQueue[i];

		CEntityKeyValuesProvider *pKeyValues = aSpawn.GetKeyValuesProvider();

		CEntityKeyValuesAttributeProvider *pAttr = (CEntityKeyValuesAttributeProvider *)pKeyValues->GetAttribute(aClassnameKey);

		if(pAttr)
		{
			const char *pszClassname = pAttr->GetValueString();

			if(pszClassname && pszClassname[0])
			{
				CBaseEntity *pEntity = (CBaseEntity *)pEntitySystem->CreateEntity(aSpawn.GetSpawnGroup(), pszClassname, ENTITY_NETWORKING_MODE_DEFAULT, iForceEdictIndex, -1, false);

				if(pEntity)
				{
					aDetails.PushFormat("Created \"%s\" (force edict index is %d, result index is %d) entity", pszClassname, iForceEdictIndex.Get(), pEntity->m_pEntity->m_EHandle.GetEntryIndex());

					pEntitySystem->QueueSpawnEntity(pEntity->m_pEntity, pKeyValues);
				}
				else
				{
					aWarnings.PushFormat("Failed to create \"%s\" entity (queue number is %d)", pszClassname, i);
				}
			}
			else
			{
				aWarnings.PushFormat("Empty entity \"%s\" key (queue number is %d)", aClassnameKey.m_pszName, i);
			}
		}
		else
		{
			aWarnings.PushFormat("Failed to get \"%s\" entity key (queue number is %d)", aClassnameKey.m_pszName, i);
		}
	}

	aDetails.Send([](const char *pszContent){
		g_pEntityManagerLogger->Message({255, 200, 200, 255}, pszContent);
	});

	aWarnings.Send([](const char *pszContent){
		g_pEntityManagerLogger->Warning({255, 255, 0, 255}, pszContent);
	});

	if(aWarnings.Count() < this->m_vecEntitySpawnQueue.Count())
	{
		pEntitySystem->ExecuteQueuedCreation();
	}

	this->ReleaseSpawnQueued();

	return iQueueLength;
}

int EntityManager::ProviderAgent::SpawnQueued(SpawnGroupHandle_t hSpawnGroup)
{
	CEntitySystemProvider *pEntitySystem = (CEntitySystemProvider *)g_pGameEntitySystem;

	auto &vecEntitySpawnQueue = this->m_vecEntitySpawnQueue;

	char sEntityError[256];

	const int iQueueLength = vecEntitySpawnQueue.Count();

	auto aDetails = g_pEntityManagerLogger->CreateDetailsScope(), 
	     aWarnings = g_pEntityManagerLogger->CreateWarningsScope();

	const CEntityIndex iForceEdictIndex = CEntityIndex(-1);

	const auto &aClassnameKey = this->GetCachedClassnameKey();

	for(int i = 0; i < vecEntitySpawnQueue.Count(); i++)
	{
		const auto &aSpawn = vecEntitySpawnQueue[i];

		if(hSpawnGroup == aSpawn.GetSpawnGroup())
		{
			CEntityKeyValuesProvider *pKeyValues = aSpawn.GetKeyValuesProvider();

			CEntityKeyValuesAttributeProvider *pAttr = (CEntityKeyValuesAttributeProvider *)pKeyValues->GetAttribute(aClassnameKey);

			if(pAttr)
			{
				const char *pszClassname = pAttr->GetValueString();

				if(pszClassname && pszClassname[0])
				{
					CBaseEntity *pEntity = (CBaseEntity *)pEntitySystem->CreateEntity(hSpawnGroup, pszClassname, ENTITY_NETWORKING_MODE_DEFAULT, iForceEdictIndex, -1, false);

					if(pEntity)
					{
						aDetails.PushFormat("Created \"%s\" (force edict index is %d, result index is %d) entity", pszClassname, iForceEdictIndex.Get(), pEntity->m_pEntity->m_EHandle.GetEntryIndex());

						pEntitySystem->QueueSpawnEntity(pEntity->m_pEntity, pKeyValues);
					}
					else
					{
						aWarnings.PushFormat("Failed to create \"%s\" entity (queue number is %d)", pszClassname, i);
					}
				}
				else
				{
					aWarnings.PushFormat("Empty entity \"%s\" key (queue number is %d)", aClassnameKey.m_pszName, i);
				}
			}
			else
			{
				aWarnings.PushFormat("Failed to get \"%s\" entity key (queue number is %d)", aClassnameKey.m_pszName, i);
			}

			vecEntitySpawnQueue.FastRemove(i);
			i--;
		}
	}

	aDetails.Send([](const char *pszContent){
		g_pEntityManagerLogger->Detailed(pszContent);
	});

	aWarnings.Send([](const char *pszContent){
		g_pEntityManagerLogger->Warning(pszContent);
	});

	const int iCompliteQueueLength = this->m_vecEntitySpawnQueue.Count();

	if(aWarnings.Count() < iCompliteQueueLength)
	{
		pEntitySystem->ExecuteQueuedCreation();
	}

	return iQueueLength - iCompliteQueueLength;
}

void EntityManager::ProviderAgent::PushDestroyQueue(CEntityInstance *pEntity)
{
	this->m_vecEntityDestroyQueue.AddToTail(pEntity);
}

void EntityManager::ProviderAgent::PushDestroyQueue(CEntityIdentity *pEntity)
{
	this->m_vecEntityDestroyQueue.AddToTail(pEntity);
}

int EntityManager::ProviderAgent::AddDestroyQueueToTail(CUtlVector<const CEntityIdentity *> *vecTarget)
{
	const auto &vecEntityDestroyQueue = this->m_vecEntityDestroyQueue;

	const int iQueueLength = vecEntityDestroyQueue.Count();

	CEntityIdentity **pEntityArr = (CEntityIdentity **)malloc(iQueueLength * sizeof(CEntityIdentity *));
	CEntityIdentity **pEntityCur = pEntityArr;

	for(int i = 0; i < iQueueLength; i++)
	{
		const auto &aDestroy = vecEntityDestroyQueue[i];

		*pEntityArr = aDestroy.GetIdnetity();
		pEntityCur++;
	}

	int iResult = vecTarget->AddMultipleToTail(((uintptr_t)pEntityCur - (uintptr_t)pEntityArr) / sizeof(decltype(pEntityArr)), pEntityArr);

	free(pEntityArr);

	return iResult;
}

void EntityManager::ProviderAgent::ReleaseDestroyQueued()
{
	this->m_vecEntityDestroyQueue.Purge();
}

int EntityManager::ProviderAgent::DestroyQueued()
{
	CEntitySystemProvider *pEntitySystem = (CEntitySystemProvider *)g_pGameEntitySystem;

	const auto &vecEntityDestroyQueue = this->m_vecEntityDestroyQueue;

	const int iQueueLength = vecEntityDestroyQueue.Count();

	for(int i = 0; i < iQueueLength; i++)
	{
		const auto &aDestroy = vecEntityDestroyQueue[i];

		pEntitySystem->QueueDestroyEntity(aDestroy.GetIdnetity());
	}

	if(iQueueLength)
	{
		pEntitySystem->ExecuteQueuedDeletion();
	}

	return iQueueLength;
}

const EntityKey &EntityManager::ProviderAgent::GetCachedEntityKey(CacheMapOIndexType nElm)
{
	return this->m_mapCachedKeys[nElm];
}

const EntityKey &EntityManager::ProviderAgent::GetCachedClassnameKey()
{
	return this->m_mapCachedKeys[this->m_nElmCachedClassnameKey];
}

EntityManager::ProviderAgent::CacheMapOIndexType EntityManager::ProviderAgent::CacheOrGetEntityKey(const char *pszName)
{
	const CUtlString sName = pszName;

	CacheMapOIndexType nFindElm = this->m_mapCachedKeys.Find(sName);

	return nFindElm == this->m_mapCachedKeys.InvalidIndex() ? this->m_mapCachedKeys.Insert(sName, MakeEntityKey(pszName, sName.Get(), sName.Length())) : nFindElm;
}

EntityManager::ProviderAgent::SpawnData::SpawnData(CEntityKeyValues *pKeyValues)
{
	this->m_pKeyValues = pKeyValues;
	this->m_hSpawnGroup = ThisClass::GetAnySpawnGroup();
}

EntityManager::ProviderAgent::SpawnData::SpawnData(CEntityKeyValues *pKeyValues, SpawnGroupHandle_t hSpawnGroup)
{
	this->m_pKeyValues = pKeyValues;
	this->m_hSpawnGroup = hSpawnGroup;
}

EntityManager::ProviderAgent::SpawnData::~SpawnData()
{
	this->Release();
}

CEntityKeyValues *EntityManager::ProviderAgent::SpawnData::GetKeyValues() const
{
	return this->m_pKeyValues;
}

EntityManager::CEntityKeyValuesProvider *EntityManager::ProviderAgent::SpawnData::GetKeyValuesProvider() const
{
	return (CEntityKeyValuesProvider *)this->m_pKeyValues;
}

SpawnGroupHandle_t EntityManager::ProviderAgent::SpawnData::GetSpawnGroup() const
{
	return this->m_hSpawnGroup;
}

SpawnGroupHandle_t EntityManager::ProviderAgent::SpawnData::GetAnySpawnGroup()
{
	return (SpawnGroupHandle_t)-1;
}

bool EntityManager::ProviderAgent::SpawnData::IsAnySpawnGroup() const
{
	return this->GetSpawnGroup() == ThisClass::GetAnySpawnGroup();
}

void EntityManager::ProviderAgent::SpawnData::Release()
{
	this->GetKeyValuesProvider()->Release();
}

EntityManager::ProviderAgent::DestoryData::DestoryData(CEntityInstance *pInstance)
{
	this->m_pIdentity = pInstance->m_pEntity;
}

EntityManager::ProviderAgent::DestoryData::DestoryData(CEntityIdentity *pIdentity)
{
	this->m_pIdentity = pIdentity;
}

EntityManager::ProviderAgent::DestoryData::~DestoryData()
{
	// Empty.
}

CEntityIdentity *EntityManager::ProviderAgent::DestoryData::GetIdnetity() const
{
	return this->m_pIdentity;
}
