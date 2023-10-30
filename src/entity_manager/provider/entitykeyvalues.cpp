#include "entitykeyvalues.h"
#include <entity_manager/provider.h>

#include <tier0/memalloc.h>
#include <entity2/entitysystem.h>

extern EntityManager::Provider *g_pEntityManagerProvider;

extern CGameEntitySystem *g_pGameEntitySystem;

CEntityKeyValues *EntityManager::CEntityKeyValuesProvider::Create(CUtlScratchMemoryPool *pMemoryPool, char eContainerType)
{
	const auto &aGameData = g_pEntityManagerProvider->GetGameDataStorage().GetEntityKeyValues();

	CEntityKeyValues *pNewKeyValues = (CEntityKeyValues *)MemAlloc_Alloc(aGameData.GetSizeof());

	if(pNewKeyValues)
	{
		(aGameData.EntityKeyValuesFunction())(pNewKeyValues, pMemoryPool, eContainerType);
		++*(uint16 *)((uintptr_t)pNewKeyValues + aGameData.GetRefCountOffset());
	}

	return pNewKeyValues;
}

void EntityManager::CEntityKeyValuesProvider::AddRef()
{
	g_pGameEntitySystem->AddRefKeyValues(this);
}

uint16 EntityManager::CEntityKeyValuesProvider::GetRefCount()
{
	return *(uint16 *)((uintptr_t)this + g_pEntityManagerProvider->GetGameDataStorage().GetEntityKeyValues().GetRefCountOffset());
}

void EntityManager::CEntityKeyValuesProvider::Release()
{
	g_pGameEntitySystem->ReleaseKeyValues(this);
}

CEntityKeyValuesAttribute *EntityManager::CEntityKeyValuesProvider::GetAttribute(const EntityKey &key, char *psValue)
{
	return (g_pEntityManagerProvider->GetGameDataStorage().GetEntityKeyValues().GetAttributeFunction())(this, key, psValue);
}

const char *EntityManager::CEntityKeyValuesAttributeProvider::GetValueString(const char *pszDefaultValue)
{
	return (g_pEntityManagerProvider->GetGameDataStorage().GetEntityKeyValues().AttributeGetValueStringFunction())(this, pszDefaultValue);
}

void EntityManager::CEntityKeyValuesProvider::SetAttributeValue(CEntityKeyValuesAttribute *pAttr, const char *pString)
{
	(g_pEntityManagerProvider->GetGameDataStorage().GetEntityKeyValues().SetAttributeValueFunction())(this, pAttr, pString);
}
