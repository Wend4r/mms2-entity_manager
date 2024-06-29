#include "provider_agent.hpp"

#include "provider.hpp"
#include "provider/entitysystem.hpp"
#include "provider/source2server.hpp"
#include <logger.hpp>

#include <ehandle.h> //FIXME: fix this one in https://github.com/Wend4r/hl2sdk/commits/cs2-entity_handle-get
#include <tier0/dbg.h>
#include <tier0/memalloc.h>
#include <tier0/platform.h>
#include <tier0/keyvalues.h>
#include <tier1/generichash.h>
#include <tier1/keyvalues3.h>

DLL_IMPORT EntityManager::Provider *g_pEntityManagerProvider;

DLL_IMPORT IServerGameDLL *server;
DLL_IMPORT IGameEventManager2 *gameeventmanager;

DLL_EXPORT CEntitySystem *g_pEntitySystem = NULL;
DLL_EXPORT CGameEntitySystem *g_pGameEntitySystem = NULL;
DLL_EXPORT CSpawnGroupMgrGameSystem *g_pSpawnGroupMgr = NULL;

class CDefOpsStringBinaryBlock
{
public:
	static bool LessFunc(const CUtlBinaryBlock &lhs, const CUtlBinaryBlock &rhs)
	{
		return StringLessThan((const char *)lhs.Get(), (const char *)rhs.Get());
	}
};

EntityManager::ProviderAgent::ProviderAgent()
 :   m_mapCachedKeys(CDefOpsStringBinaryBlock::LessFunc)
{
	// Cache the classname.
	{
		static const char szClassname[] = "classname";

		this->m_nElmCachedClassnameKey = this->m_mapCachedKeys.Insert({szClassname, sizeof(szClassname)}, {szClassname, (const char *)szClassname});
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
	this->ReleaseSpawnGroups();
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
	return (g_pEntitySystem = (CEntitySystem *)(g_pGameEntitySystem = *(CGameEntitySystem **)((uintptr_t)g_pGameResourceServiceServer + g_pEntityManagerProvider->GetGameDataStorage().GetGameResource().GetEntitySystemOffset()))) != NULL;
}

bool EntityManager::ProviderAgent::NotifyGameEventsUpdated()
{
	return true;
}

bool EntityManager::ProviderAgent::NotifySpawnGroupMgrUpdated()
{
	return (g_pSpawnGroupMgr = *g_pEntityManagerProvider->GetGameDataStorage().GetSpawnGroup().GetSpawnGroupMgrAddress()) != NULL;
}

bool EntityManager::ProviderAgent::ErectResourceManifest(ISpawnGroup *pSpawnGroup, int nCount, const EntitySpawnInfo_t *pEntities, const matrix3x4a_t *const vWorldOffset)
{
	return this->m_aResourceManifest.Erect(pSpawnGroup, nCount, pEntities, vWorldOffset);
}

IEntityResourceManifest *EntityManager::ProviderAgent::GetMyEntityManifest()
{
	return this->m_aResourceManifest.GetEntityPart();
}

bool EntityManager::ProviderAgent::CreateSpawnGroup(const SpawnGroupDesc_t &aDesc, const Vector &vecLandmarkOffset)
{
	SpawnGroup *aSpawnGroup = new SpawnGroup();

	bool bResult = aSpawnGroup->Start(aDesc, vecLandmarkOffset);

	if(bResult)
	{
		this->m_vecSpawnGroups.AddToTail(aSpawnGroup);
	}

	return bResult;
}

void EntityManager::ProviderAgent::ReleaseSpawnGroups()
{
	this->m_vecSpawnGroups.PurgeAndDeleteElements();
}

void EntityManager::ProviderAgent::NotifyAllocateSpawnGroup(SpawnGroupHandle_t handle, ISpawnGroup *pSpawnGroup)
{
	const char *pWorldName = pSpawnGroup->GetWorldName();

	if(pWorldName && pWorldName[0])
	{
		for(int i = 0; i < this->m_vecSpawnGroups.Count(); i++)
		{
			SpawnGroup *pSpawnGroupAgent = this->m_vecSpawnGroups[i];

			const char *pLevelName = pSpawnGroupAgent->GetLevelName();

			if(pLevelName && pLevelName[0] && !V_strcmp(pWorldName, pLevelName))
			{
				pSpawnGroupAgent->NotifyAllocateSpawnGroup(handle, pSpawnGroup);
			}
		}
	}
}

void EntityManager::ProviderAgent::NotifyDestroySpawnGroup(SpawnGroupHandle_t handle)
{
	for(int i = 0; i < this->m_vecSpawnGroups.Count(); i++)
	{
		SpawnGroup *pSpawnGroupAgent = this->m_vecSpawnGroups[i];

		if(handle == pSpawnGroupAgent->GetAllocatedSpawnGroup())
		{
			pSpawnGroupAgent->NotifyDestroySpawnGroup(handle);

			delete pSpawnGroupAgent;
			this->m_vecSpawnGroups.FastRemove(i);
		}
	}
}

void EntityManager::ProviderAgent::PushSpawnQueueOld(KeyValues *pOldOne, SpawnGroupHandle_t hSpawnGroup, Logger::Scope *pWarnings)
{
	CEntityKeyValues *pNewKeyValues = new CEntityKeyValues(/* (CEntitySystemProvider *)g_pGameEntitySystem)->GetKeyValuesContextAllocator(), EKV_ALLOCATOR_EXTERNAL */ &this->m_aEntityAllocator, EKV_ALLOCATOR_EXTERNAL);

	// Parse attributes.
	{
		const char *pszAttrSectionKey = "attributes";

		KeyValues *pAttributeValues = pOldOne->FindKey(pszAttrSectionKey, false);

		if(pAttributeValues)
		{
			FOR_EACH_VALUE(pAttributeValues, pAttrKeyValue)
			{
				const char *pszAttrKey = pAttrKeyValue->GetName();

				const EntityKeyId_t &aAttrKey = this->GetCachedEntityKey(this->CacheOrGetEntityKey(pszAttrKey));

				pNewKeyValues->SetString(aAttrKey, pAttrKeyValue->GetString(), true);
			}

			pOldOne->RemoveSubKey(pAttributeValues, true, true);
		}
	}

	FOR_EACH_VALUE(pOldOne, pKeyValue)
	{
		const char *pszKey = pKeyValue->GetName();

		const EntityKeyId_t &aKey = this->GetCachedEntityKey(this->CacheOrGetEntityKey(pszKey));

		pNewKeyValues->SetString(aKey, pKeyValue->GetString());
	}

	this->PushSpawnQueue(pNewKeyValues, hSpawnGroup);
}

void EntityManager::ProviderAgent::PushSpawnQueue(CEntityKeyValues *pKeyValues, SpawnGroupHandle_t hSpawnGroup)
{
	g_pGameEntitySystem->AddRefKeyValues(pKeyValues);

	this->m_vecEntitySpawnQueue.AddToTail({pKeyValues, hSpawnGroup});
}

int EntityManager::ProviderAgent::AddSpawnQueueToTail(CUtlVector<const CEntityKeyValues *> &vecTarget, SpawnGroupHandle_t hSpawnGroup)
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

				g_pGameEntitySystem->AddRefKeyValues(pKeyValues);

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

					g_pGameEntitySystem->AddRefKeyValues(pKeyValues);

					*ppKeyValuesCur = pKeyValues;
					ppKeyValuesCur++;
				}
			}
		}
	}

	int iResult = vecTarget.AddMultipleToTail(iQueueLength, ppKeyValuesArr);

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

bool EntityManager::ProviderAgent::HasInSpawnQueue(SpawnGroupHandle_t hSpawnGroup)
{
	FOR_EACH_VEC(this->m_vecEntitySpawnQueue, i)
	{
		const auto &aSpawn = this->m_vecEntitySpawnQueue[i];

		if(aSpawn.GetSpawnGroup() == hSpawnGroup)
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

	const int iQueueLength = vecEntitySpawnQueue.Count();

	const CEntityIndex iForceEdictIndex = CEntityIndex(-1);

	const auto &aClassnameKey = this->GetCachedClassnameKey();

	for(int i = 0; i < iQueueLength; i++)
	{
		const auto &aSpawn = vecEntitySpawnQueue[i];

		if(hSpawnGroup == SpawnData::GetAnySpawnGroup() || hSpawnGroup == aSpawn.GetSpawnGroup())
		{
			CEntityKeyValues *pKeyValues = aSpawn.GetKeyValues();

			if(pKeyValues)
			{
				const char *pszClassname = pKeyValues->GetString(aClassnameKey);

				if(pszClassname && pszClassname[0])
				{
					CEntityInstance *pEntity = pEntitySystem->CreateEntity(aSpawn.GetSpawnGroup(), pszClassname, ENTITY_NETWORKING_MODE_DEFAULT, iForceEdictIndex, -1, false);

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

int EntityManager::ProviderAgent::AddDestroyQueueToTail(CUtlVector<const CEntityIdentity *> &vecTarget)
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

	int iResult = vecTarget.AddMultipleToTail(((uintptr_t)pEntityCur - (uintptr_t)pEntityArr) / sizeof(decltype(pEntityArr)), pEntityArr);

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

bool EntityManager::ProviderAgent::DumpOldKeyValues(KeyValues *pOldOne, Logger::Scope &aOutput, Logger::Scope *paWarnings)
{
	FOR_EACH_VALUE(pOldOne, pKeyValue)
	{
		const char *pszName = pKeyValue->GetName(), 
		           *pszValue = pKeyValue->GetString();

		if(V_stristr(pszName, "color"))
		{
			Color rgba = pKeyValue->GetColor();

			ProviderAgent::MakeDumpColorAlpha(rgba);
			aOutput.PushFormat(rgba, "\"%s\" \"%s\"", pszName, pszValue);
		}
		else
		{
			aOutput.PushFormat("\"%s\" \"%s\"", pszName, pszValue);
		}
	}

	return true;
}

bool EntityManager::ProviderAgent::DumpEntityKeyValues(const CEntityKeyValues *pKeyValues, DumpEntityKeyValuesFlags_t eFlags, Logger::Scope &aOutput, Logger::Scope *paWarnings)
{
	bool bResult = pKeyValues != nullptr;

	if(bResult)
	{
		const KeyValues3 *pRoot = *(const KeyValues3 **)((uintptr_t)pKeyValues + /* offsetof(CEntityKeyValues, m_pKeyValues) */ 2 * sizeof(void *));

		bResult = pRoot != nullptr;

		if(bResult)
		{
			for(int i = 0, iMemberCount = pRoot->GetMemberCount(); i < iMemberCount; i++)
			{ 
				const char *pszName = pRoot->GetMemberName(i);

				KeyValues3 *pMember = const_cast<KeyValues3 *>(pRoot->GetMember(i));

				if(pMember)
				{
					char sValue[512];

					int iStoredLength = This::DumpEntityKeyValue(pMember, sValue, sizeof(sValue));

					if(iStoredLength)
					{
						if(eFlags)
						{
							if(eFlags & This::DEKVF_TYPE)
							{
								V_snprintf(&sValue[iStoredLength], sizeof(sValue) - iStoredLength, " // Type is #%d", pMember->GetTypeEx());
							}

							if(eFlags & This::DEKVF_SUBTYPE)
							{
								V_snprintf(&sValue[iStoredLength], sizeof(sValue) - iStoredLength, " // SubType is #%d", pMember->GetSubType());
							}
						}

						if(V_stristr(pszName, "color"))
						{
							Color rgba = pMember->GetColor();

							MakeDumpColorAlpha(rgba);
							aOutput.PushFormat(rgba, "%s = %s", pszName, sValue);
						}
						else
						{
							aOutput.PushFormat("%s = %s", pszName, sValue);
						}
					}
					else
					{
						aOutput.PushFormat("// \"%s\" is empty", pszName);
					}
				}
				else if(paWarnings)
				{
					paWarnings->PushFormat("Failed to get \"%s\" key member", pszName);
				}
			}

			const KeyValues3 *pAttributes = *(const KeyValues3 **)((uintptr_t)pKeyValues + /* offsetof(CEntityKeyValues, m_pAttibutes) */ 3 * sizeof(void *));

			if(pAttributes)
			{
				const int iMemberCount = pAttributes->GetMemberCount();
				
				if(iMemberCount)
				{
					aOutput.PushFormat("%s = ", "attributes");
					aOutput.Push("{");

					for(int i = 0; i < iMemberCount; i++)
					{ 
						const char *pszAttrName = pAttributes->GetMemberName(i);

						KeyValues3 *pMember = const_cast<KeyValues3 *>(pAttributes->GetMember(i));

						if(pMember)
						{
							char sValue[512];

							int iStoredLength = This::DumpEntityKeyValue(pMember, sValue, sizeof(sValue));

							if(iStoredLength)
							{
								V_snprintf(&sValue[iStoredLength], sizeof(sValue) - iStoredLength, " // Type is #%d | SubType is #%d", pMember->GetTypeEx(), pMember->GetSubType());

								if(V_stristr(pszAttrName, "color"))
								{
									Color rgba = pMember->GetColor();

									MakeDumpColorAlpha(rgba);
									aOutput.PushFormat(rgba, "\t%s = %s", pszAttrName, sValue);
								}
								else
								{
									aOutput.PushFormat("\t%s = %s", pszAttrName, sValue);
								}
							}
							else
							{
								aOutput.PushFormat("\t// \"%s\" attribute is empty", pszAttrName);
							}
						}
						else if(paWarnings)
						{
							paWarnings->PushFormat("Failed to get \"%s\" key member of attribute", pszAttrName);
						}
					}

					aOutput.Push("}");
				}
			}
		}
	}
	else if(paWarnings)
	{
		paWarnings->Push("Skip an entity without key values");
	}

	return bResult;
}

int EntityManager::ProviderAgent::DumpEntityKeyValue(KeyValues3 *pMember, char *psBuffer, size_t nMaxLength)
{
	const char *pDest = "unknown";

	switch(pMember->GetTypeEx())
	{
		case KV3_TYPEEX_NULL:
		{
			const char sNullable[] = "null";

			size_t nStoreSize = sizeof(sNullable);

			V_strncpy(psBuffer, (const char *)sNullable, nMaxLength);

			return nMaxLength < nStoreSize ? nMaxLength : nStoreSize;
		}

		case KV3_TYPEEX_BOOL:
		{
			return V_snprintf(psBuffer, nMaxLength, "%s", pMember->GetBool() ? "true" : "false");
		}

		case KV3_TYPEEX_INT:
			switch(pMember->GetSubType())
			{
				case KV3_SUBTYPE_INT8:
					return V_snprintf(psBuffer, nMaxLength, "%hhd", pMember->GetInt8());
				case KV3_SUBTYPE_INT16:
					return V_snprintf(psBuffer, nMaxLength, "%hd", pMember->GetShort());
				case KV3_SUBTYPE_INT32:
					return V_snprintf(psBuffer, nMaxLength, "%d", pMember->GetInt());
				case KV3_SUBTYPE_INT64:
					return V_snprintf(psBuffer, nMaxLength, "%lld", pMember->GetInt64());
				case KV3_SUBTYPE_EHANDLE:
				{
					const CEntityHandle &aHandle = pMember->GetEHandle();

					int iIndex = -1;

					const char *pszClassname = nullptr;

					if(aHandle.IsValid())
					{
						CEntityInstance *pEntity = aHandle.Get();

						if(pEntity)
						{
							iIndex = pEntity->GetEntityIndex().Get();
							pszClassname = pEntity->GetClassname();
						}
						else
						{
							iIndex = aHandle.GetEntryIndex();
						}
					}

					return V_snprintf(psBuffer, nMaxLength, pszClassname && pszClassname[0] ? "\"entity:%d:%s\"" : "\"entity:%d\"", iIndex, pszClassname);
				}
				default:
					AssertMsg1(0, "KV3: Unrealized int subtype is %d\n", pMember->GetSubType());
					return 0;
			}
			break;
		case KV3_TYPEEX_UINT:
			switch(pMember->GetSubType())
			{
				case KV3_SUBTYPE_UINT8:
					return V_snprintf(psBuffer, nMaxLength, "%hhu", pMember->GetUInt8());
				case KV3_SUBTYPE_UINT16:
					return V_snprintf(psBuffer, nMaxLength, "%hu", pMember->GetUShort());
				case KV3_SUBTYPE_UINT32:
					return V_snprintf(psBuffer, nMaxLength, "%u", pMember->GetUInt());
				case KV3_SUBTYPE_UINT64:
					return V_snprintf(psBuffer, nMaxLength, "%llu", pMember->GetUInt64());
				case KV3_SUBTYPE_STRING_TOKEN:
				{
					const CUtlStringToken &aToken = pMember->GetStringToken();

					return V_snprintf(psBuffer, nMaxLength, "\"string_token:0x%x\"", aToken.GetHashCode());
				}
				case KV3_SUBTYPE_EHANDLE:
				{
					const CEntityHandle &aHandle = pMember->GetEHandle();

					int iIndex = -1;

					const char *pszClassname = nullptr;

					if(aHandle.IsValid())
					{
						CEntityInstance *pEntity = aHandle.Get();

						if(pEntity)
						{
							iIndex = pEntity->GetEntityIndex().Get();
							pszClassname = pEntity->GetClassname();
						}
						else
						{
							iIndex = aHandle.GetEntryIndex();
						}
					}

					return V_snprintf(psBuffer, nMaxLength, pszClassname && pszClassname[0] ? "\"entity:%d:%s\"" : "\"entity:%d\"", iIndex, pszClassname);
				}
				default:
					AssertMsg1(0, "KV3: Unrealized uint subtype is %d\n", pMember->GetSubType());
					return 0;
			}
			break;
		case KV3_TYPEEX_DOUBLE:
			switch(pMember->GetSubType())
			{
				case KV3_SUBTYPE_FLOAT32:
					return V_snprintf(psBuffer, nMaxLength, "%f", pMember->GetFloat());
				case KV3_SUBTYPE_FLOAT64:
					return V_snprintf(psBuffer, nMaxLength, "%g", pMember->GetDouble());
				default:
				{
					AssertMsg1(0, "KV3: Unrealized double subtype is %d\n", pMember->GetSubType());
					return 0;
				}
			}
			break;
		case KV3_TYPEEX_STRING:
		case KV3_TYPEEX_STRING_SHORT:
		case KV3_TYPEEX_STRING_EXTERN:
		case KV3_TYPEEX_BINARY_BLOB:
		case KV3_TYPEEX_BINARY_BLOB_EXTERN:
			return V_snprintf(psBuffer, nMaxLength, "\"%s\"", pMember->GetString());
		case KV3_TYPEEX_ARRAY:
		case KV3_TYPEEX_ARRAY_FLOAT32:
		case KV3_TYPEEX_ARRAY_FLOAT64:
		case KV3_TYPEEX_ARRAY_INT16:
		case KV3_TYPEEX_ARRAY_INT32:
		case KV3_TYPEEX_ARRAY_UINT8_SHORT:
		case KV3_TYPEEX_ARRAY_INT16_SHORT:
			switch(pMember->GetSubType())
			{
				case KV3_SUBTYPE_ARRAY:
				{
					int iBufCur = 0;

					if(nMaxLength > 2)
					{
						KeyValues3 **pArray = pMember->GetArrayBase();

						const int iArrayLength = pMember->GetArrayElementCount();

						if(iArrayLength > 0)
						{
							psBuffer[0] = '[';
							iBufCur = 1;

							int i = 0;

							char *pArrayBuf = (char *)stackalloc(nMaxLength);

							do
							{
								if(ProviderAgent::DumpEntityKeyValue(pArray[i], pArrayBuf, nMaxLength))
								{
									iBufCur += V_snprintf(&psBuffer[iBufCur], nMaxLength - iBufCur, "%s, ", pArrayBuf);
								}

								i++;
							}
							while(i < iArrayLength);

							iBufCur -= 2; // Strip ", " of end.
							psBuffer[iBufCur++] = ']';
							psBuffer[iBufCur] = '\0';
						}
					}

					return iBufCur;
				}

				case KV3_SUBTYPE_VECTOR:
				case KV3_SUBTYPE_ROTATION_VECTOR:
				case KV3_SUBTYPE_QANGLE:
				{
					const Vector &aVec3 = pMember->GetVector();

					return V_snprintf(psBuffer, nMaxLength, "\"%f %f %f\"", aVec3.x, aVec3.y, aVec3.z);
				}
				case KV3_SUBTYPE_VECTOR2D:
				{
					const Vector2D &aVec2 = pMember->GetVector2D();

					return V_snprintf(psBuffer, nMaxLength, "\"%f %f\"", aVec2.x, aVec2.y);
				}
				case KV3_SUBTYPE_VECTOR4D:
				case KV3_SUBTYPE_QUATERNION:
				{
					const Vector4D &aVec4 = pMember->GetVector4D();

					return V_snprintf(psBuffer, nMaxLength, "\"%f %f %f %f\"", aVec4.x, aVec4.y, aVec4.z, aVec4.w);
				}
				case KV3_SUBTYPE_MATRIX3X4:
				{
					const matrix3x4_t &aMatrix3x4 = pMember->GetMatrix3x4();

					return V_snprintf(psBuffer, nMaxLength, "[\"%f %f %f\", \"%f %f %f\", \"%f %f %f\", \"%f %f %f\"]",  
					                                        aMatrix3x4.m_flMatVal[0][0], aMatrix3x4.m_flMatVal[0][1], aMatrix3x4.m_flMatVal[0][2], aMatrix3x4.m_flMatVal[0][3], 
					                                        aMatrix3x4.m_flMatVal[1][0], aMatrix3x4.m_flMatVal[1][1], aMatrix3x4.m_flMatVal[1][2], aMatrix3x4.m_flMatVal[1][3], 
					                                        aMatrix3x4.m_flMatVal[2][0], aMatrix3x4.m_flMatVal[2][1], aMatrix3x4.m_flMatVal[2][2], aMatrix3x4.m_flMatVal[2][3]);
				}
				default:
					AssertMsg1(0, "KV3: Unrealized array subtype is %d", pMember->GetSubType());
					return 0;
			}
			break;
		default:
			AssertMsg1(0, "KV3: Unrealized typeex is %d", pMember->GetTypeEx());
			return 0;
	}

	return 0;
}


bool EntityManager::ProviderAgent::MakeDumpColorAlpha(Color &rgba)
{
	bool bResult = !rgba[3];

	if(bResult)
	{
		rgba[3] = 127 + (127 * ((rgba[0] + rgba[1] + rgba[2]) / (255 * 3)));
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
	const CUtlBinaryBlock aName(pszName, V_strlen(pszName) + 1);

	CacheMapOIndexType nFindElm = this->m_mapCachedKeys.Find(aName);

	return nFindElm == this->m_mapCachedKeys.InvalidIndex() ? this->m_mapCachedKeys.Insert(aName, {(const char *)aName.Get()}) : nFindElm;
}

EntityManager::ProviderAgent::SpawnData::SpawnData(CEntityKeyValues *pKeyValues)
{
	pKeyValues->AddRef();
	this->m_pKeyValues = pKeyValues;
	this->m_hSpawnGroup = ThisClass::GetAnySpawnGroup();
}

EntityManager::ProviderAgent::SpawnData::SpawnData(CEntityKeyValues *pKeyValues, SpawnGroupHandle_t hSpawnGroup)
{
	pKeyValues->AddRef();
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
	this->GetKeyValues()->Release();
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
