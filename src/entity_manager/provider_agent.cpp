#include "provider_agent.h"

#include "provider.h"
#include "provider/entitysystem.h"
#include <logger.h>

#include <tier0/dbg.h>
#include <tier0/memalloc.h>
#include <tier0/platform.h>
#include <tier0/keyvalues.h>
#include <tier1/generichash.h>
#include <tier1/keyvalues3.h>

extern EntityManager::Provider *g_pEntityManagerProvider;

extern IGameResourceServiceServer *g_pGameResourceServiceServer;
CGameEntitySystem *g_pGameEntitySystem = NULL;
CSpawnGroupMgrGameSystem *g_pSpawnGroupMgr = NULL;

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

		this->m_nElmCachedClassnameKey = this->m_mapCachedKeys.Insert(szClassname, szClassname);
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

bool EntityManager::ProviderAgent::NotifyGameResourceUpdated()
{
	bool bResult = this->m_aResourceManifest.Reinit(RESOURCE_MANIFEST_LOAD_STREAMING_DATA, __FUNCTION__, RESOURCE_MANIFEST_LOAD_PRIORITY_HIGH /* Run-time entities update at players view */);

	if(bResult)
	{
		// ...
	}

	return bResult;
}

bool EntityManager::ProviderAgent::NotifyEntitySystemUpdated()
{
	return (g_pGameEntitySystem = *(CGameEntitySystem **)((uintptr_t)g_pGameResourceServiceServer + g_pEntityManagerProvider->GetGameDataStorage().GetGameResource().GetEntitySystemOffset())) != NULL;
}

bool EntityManager::ProviderAgent::NotifySpawnGroupMgrUpdated()
{
	return (g_pSpawnGroupMgr = *g_pEntityManagerProvider->GetGameDataStorage().GetSpawnGroup().GetSpawnGroupMgrAddress()) != NULL;
}

bool EntityManager::ProviderAgent::ErectResourceManifest(SpawnGroupHandle_t hSpawnGroup, int nCount, const EntitySpawnInfo_t *pEntities, const matrix3x4a_t *const vWorldOffset)
{
	return this->m_aResourceManifest.Erect(hSpawnGroup, nCount, pEntities, vWorldOffset);
}

IEntityResourceManifest *EntityManager::ProviderAgent::GetMyEntityManifest()
{
	return this->m_aResourceManifest.GetEntityPart();
}

void EntityManager::ProviderAgent::PushSpawnQueueOld(KeyValues *pOldKeyValues, SpawnGroupHandle_t hSpawnGroup, Logger::Scope *pDetails, Logger::Scope *pWarnings)
{
	int iNewIndex = this->m_vecEntitySpawnQueue.Count();

	CEntityKeyValuesProvider *pNewKeyValues = (CEntityKeyValuesProvider *)CEntityKeyValuesProvider::Create(((CEntitySystemProvider *)g_pGameEntitySystem)->GetKeyValuesClusterAllocator(), 3);

	Color rgbaPrev = LOGGER_COLOR_MESSAGE;

	if(pDetails)
	{
		rgbaPrev = pDetails->GetColor();
		pDetails->SetColor(pOldKeyValues->GetColor("color", rgbaPrev));
		pDetails->PushFormat("-- Queue entity #%d --", iNewIndex);
	}

	FOR_EACH_VALUE(pOldKeyValues, pKeyValue)
	{
		const char *pszKey = pKeyValue->GetName();

		CEntityKeyValuesAttribute *pAttr = pNewKeyValues->GetAttribute(this->GetCachedEntityKey(this->CacheOrGetEntityKey(pszKey)));

		if(pAttr)
		{
			const char *pszValue = pKeyValue->GetString();

			if(pDetails)
			{
				pDetails->PushFormat("\t\"%s\" has \"%s\" value", pszKey, pszValue);
			}

			pNewKeyValues->SetAttributeValue(pAttr, pszValue);
		}
		else if(pWarnings)
		{
			pWarnings->PushFormat("Failed to get \"%s\" attribute", pszKey);
		}
	}

	if(pDetails)
	{
		pDetails->SetColor(rgbaPrev);
	}

	this->PushSpawnQueue(pNewKeyValues, hSpawnGroup);
}

void EntityManager::ProviderAgent::PushSpawnQueue(CEntityKeyValues *pKeyValues, SpawnGroupHandle_t hSpawnGroup)
{
	((CEntityKeyValuesProvider *)pKeyValues)->AddRef();

	this->m_vecEntitySpawnQueue.AddToTail({pKeyValues, hSpawnGroup});
}

int EntityManager::ProviderAgent::AddSpawnQueueToTail(CUtlVector<const CEntityKeyValues *> *&pvecTarget, SpawnGroupHandle_t hSpawnGroup)
{
	const auto &vecEntitySpawnQueue = this->m_vecEntitySpawnQueue;

	int iQueueLength = vecEntitySpawnQueue.Count();

	CEntityKeyValues **ppKeyValuesArr = (CEntityKeyValues **)MemAlloc_Alloc(iQueueLength * sizeof(CEntityKeyValues *));

	// Fill ppKeyValuesArr from .
	{
		CEntityKeyValues **ppKeyValuesCur = ppKeyValuesArr;

		if(hSpawnGroup == SpawnData::GetAnySpawnGroup())
		{
			for(int i = 0; i < iQueueLength; i++)
			{
				CEntityKeyValues *pKeyValues = vecEntitySpawnQueue[i].GetKeyValues();

				((CEntityKeyValuesProvider *)pKeyValues)->AddRef();

				*ppKeyValuesCur = pKeyValues;
				ppKeyValuesCur++;
			}
		}
		else
		{
			for(int i = 0; i < iQueueLength; i++)
			{
				const auto &aSpawn = vecEntitySpawnQueue[i];

				if(hSpawnGroup == aSpawn.GetSpawnGroup())
				{
					CEntityKeyValues *pKeyValues = aSpawn.GetKeyValues();

					((CEntityKeyValuesProvider *)pKeyValues)->AddRef();

					*ppKeyValuesCur = pKeyValues;
					ppKeyValuesCur++;
				}
			}
		}
	}

	if(!pvecTarget)
	{
		pvecTarget = new CUtlVector<const CEntityKeyValues *>();
	}

	int iResult = pvecTarget->AddMultipleToTail(iQueueLength, ppKeyValuesArr);

	MemAlloc_Free(ppKeyValuesArr);

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

int EntityManager::ProviderAgent::ReleaseSpawnQueued(SpawnGroupHandle_t hSpawnGroup)
{
	auto &vecEntitySpawnQueue = this->m_vecEntitySpawnQueue;

	const int iQueueLengthBefore = vecEntitySpawnQueue.Count();

	// Find and fast remove.
	{
		if(hSpawnGroup == SpawnData::GetAnySpawnGroup())
		{
			vecEntitySpawnQueue.RemoveAll();
		}
		else
		{
			for(int i = 0; i < vecEntitySpawnQueue.Count(); i++)
			{
				auto &aSpawn = vecEntitySpawnQueue[i];

				if(hSpawnGroup == aSpawn.GetSpawnGroup())
				{
					vecEntitySpawnQueue.FastRemove(i);
					i--;
				}
			}
		}
	}

	return iQueueLengthBefore - vecEntitySpawnQueue.Count();
}

int EntityManager::ProviderAgent::SpawnQueued(SpawnGroupHandle_t hSpawnGroup, Logger::Scope *pDetails, Logger::Scope *pWarnings)
{
	CEntitySystemProvider *pEntitySystem = (CEntitySystemProvider *)g_pGameEntitySystem;

	auto &vecEntitySpawnQueue = this->m_vecEntitySpawnQueue;

	char psClassname[128] = "\0", 
	     sEntityError[256];

	const int iQueueLength = vecEntitySpawnQueue.Count();

	const CEntityIndex iForceEdictIndex = CEntityIndex(-1);

	const auto &aClassnameKey = this->GetCachedClassnameKey();

	for(int i = 0; i < iQueueLength; i++)
	{
		const auto &aSpawn = vecEntitySpawnQueue[i];

		if(hSpawnGroup == SpawnData::GetAnySpawnGroup() || hSpawnGroup == aSpawn.GetSpawnGroup())
		{
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
						if(pDetails)
						{
							pDetails->PushFormat("Created \"%s\" (force edict index is %d, result index is %d) entity", pszClassname, iForceEdictIndex.Get(), pEntity->m_pEntity->m_EHandle.GetEntryIndex());
						}

						pEntitySystem->QueueSpawnEntity(pEntity->m_pEntity, pKeyValues);
					}
					else if(pWarnings)
					{
						pWarnings->PushFormat("Failed to create \"%s\" entity (queue number is %d)", pszClassname, i);
					}
				}
				else if(pWarnings)
				{
					pWarnings->PushFormat("Empty entity \"%s\" key (queue number is %d)", aClassnameKey.GetString(), i);
				}
			}
			else if(pWarnings)
			{
				pWarnings->PushFormat("Failed to get \"%s\" entity key (queue number is %d)", aClassnameKey.GetString(), i);
			}
		}
	}

	if(pWarnings && pWarnings->Count() < this->m_vecEntitySpawnQueue.Count())
	{
		pEntitySystem->ExecuteQueuedCreation();
	}

	this->ReleaseSpawnQueued();

	return iQueueLength;
}

void EntityManager::ProviderAgent::PushDestroyQueue(CEntityInstance *pEntity)
{
	this->m_vecEntityDestroyQueue.AddToTail(pEntity);
}

void EntityManager::ProviderAgent::PushDestroyQueue(CEntityIdentity *pEntity)
{
	this->m_vecEntityDestroyQueue.AddToTail(pEntity->m_pInstance);
}

int EntityManager::ProviderAgent::AddDestroyQueueToTail(CUtlVector<const CEntityIdentity *> *&pvecTarget)
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

	if(!pvecTarget)
	{
		pvecTarget = new CUtlVector<const CEntityIdentity *>();
	}

	int iResult = pvecTarget->AddMultipleToTail(((uintptr_t)pEntityCur - (uintptr_t)pEntityArr) / sizeof(decltype(pEntityArr)), pEntityArr);

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

	auto &vecEntityDestroyQueue = this->m_vecEntityDestroyQueue;

	const int iQueueLength = vecEntityDestroyQueue.Count();

	if(iQueueLength)
	{
		int i = 0;

		do
		{
			pEntitySystem->QueueDestroyEntity(vecEntityDestroyQueue[i].GetIdnetity());
			i++;
		}
		while(i < iQueueLength);

		pEntitySystem->ExecuteQueuedDeletion();
		vecEntityDestroyQueue.Purge();
	}

	return iQueueLength;
}

bool EntityManager::ProviderAgent::DumpEntityKeyValues(const CEntityKeyValues *pKeyValues, Logger::Scope &aOutput, Logger::Scope *paWarnings)
{
	const CEntityKeyValuesProvider *pProvider = (const CEntityKeyValuesProvider *)pKeyValues;

	bool bResult = pProvider != nullptr;

	if(bResult)
	{
		bResult = pProvider->GetRefCount() != 0;

		if(bResult)
		{
			uint8 nType = pProvider->GetContainerType();

			bResult = !nType || nType == 3;

			if(bResult)
			{
				const KeyValues3 *pRoot = pProvider->Root();

				bResult = pRoot != nullptr;

				if(bResult)
				{
					for(int i = 0, iMemberCount = pRoot->GetMemberCount(); i < iMemberCount; i++)
					{
						const char *pAttrName = pRoot->GetMemberName(i);

						KeyValues3 *pMember = pRoot->GetMember(i);

						if(pMember)
						{
							const char *pDest = "unknown";

							switch(pMember->GetTypeEx())
							{
								case KV3_TYPEEX_NULL:
									aOutput.PushFormat("%s = null", pAttrName);
									break;
								case KV3_TYPEEX_BOOL:
									aOutput.PushFormat("%s = %s", pAttrName, pMember->GetBool() ? "true" : "false");
									break;
								case KV3_TYPEEX_INT:
									aOutput.PushFormat("%s = %lld", pAttrName, pMember->GetInt64());
									break;
								case KV3_TYPEEX_UINT:
									aOutput.PushFormat("%s = %llu", pAttrName, pMember->GetUInt64());
									break;
								case KV3_TYPEEX_DOUBLE:
									aOutput.PushFormat("%s = %g", pAttrName, pMember->GetDouble());
									break;
								case KV3_TYPEEX_STRING:
								case KV3_TYPEEX_STRING_EXTERN:
								case KV3_TYPEEX_STRING_SHORT:
									aOutput.PushFormat("%s = \"%s\"", pAttrName, pMember->GetString());
									break;
								default:
									Assert( 0 );
									aOutput.PushFormat("%s = unknown", pAttrName, pMember->GetString());
									break;
							}
						}
						else if(paWarnings)
						{
							paWarnings->PushFormat("Failed to get \"%s\" key member", pAttrName);
						}
					}
				}
				else if(paWarnings)
				{
					paWarnings->Push("Entity key values hasn't table");
				}
			}
			else if(paWarnings)
			{
				paWarnings->PushFormat(LOGGER_COLOR_ERROR, "Invalid entity key values. It is an uninitialized? (dest is %p, unknown type is %d)", pProvider, nType);
			}
		}
		else if(paWarnings)
		{
			paWarnings->PushFormat("Skip ref-empty entity key values (dest is %p)", pProvider);
		}
	}
	else if(paWarnings)
	{
		paWarnings->Push("Skip an entity without key values");
	}

	return bResult;
}

const EntityKeyId_t &EntityManager::ProviderAgent::GetCachedEntityKey(CacheMapOIndexType nElm)
{
	return this->m_mapCachedKeys[nElm];
}

const EntityKeyId_t &EntityManager::ProviderAgent::GetCachedClassnameKey()
{
	return this->m_mapCachedKeys[this->m_nElmCachedClassnameKey];
}

EntityManager::ProviderAgent::CacheMapOIndexType EntityManager::ProviderAgent::CacheOrGetEntityKey(const char *pszName)
{
	const CUtlString sName(pszName, V_strlen(pszName) + 1);

	CacheMapOIndexType nFindElm = this->m_mapCachedKeys.Find(sName);

	return nFindElm == this->m_mapCachedKeys.InvalidIndex() ? this->m_mapCachedKeys.Insert(sName, MakeStringToken(sName)) : nFindElm;
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
	return INVALID_SPAWN_GROUP;
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
