#include "entitykeyvalues.h"
#include "provider.h"

#include <tier0/memalloc.h>
#include <entity2/entitysystem.h>

extern EntityManager::Provider *g_pEntityManagerProvider;

extern CGameEntitySystem *g_pGameEntitySystem;

CEntityKeyValues *EntityManager::CEntityKeyValuesProvider::Create(CKeyValues3Context *pClusterAllocator, char eContextType)
{
	const auto &aGameData = g_pEntityManagerProvider->GetGameDataStorage().GetEntityKeyValues();

	CEntityKeyValues *pNewKeyValues = (CEntityKeyValues *)MemAlloc_Alloc(aGameData.GetSizeof());

	if(pNewKeyValues)
	{
		(aGameData.EntityKeyValuesFunction())(pNewKeyValues, pClusterAllocator, eContextType);
		++*(uint16 *)((uintptr_t)pNewKeyValues + aGameData.GetRefCountOffset());
	}

	return pNewKeyValues;
}

KeyValues3 *EntityManager::CEntityKeyValuesProvider::Root()
{
	return *(KeyValues3 **)((uintptr_t)this + g_pEntityManagerProvider->GetGameDataStorage().GetEntityKeyValues().GetRootOffset());
}

const KeyValues3 *EntityManager::CEntityKeyValuesProvider::Root() const
{
	return *(const KeyValues3 **)((uintptr_t)this + g_pEntityManagerProvider->GetGameDataStorage().GetEntityKeyValues().GetRootOffset());
}

void EntityManager::CEntityKeyValuesProvider::AddRef()
{
	g_pGameEntitySystem->AddRefKeyValues(this);
}

uint16 EntityManager::CEntityKeyValuesProvider::GetRefCount() const
{
	return *(uint16 *)((uintptr_t)this + g_pEntityManagerProvider->GetGameDataStorage().GetEntityKeyValues().GetRefCountOffset());
}

uint8 EntityManager::CEntityKeyValuesProvider::GetContainerType() const
{
	return *(uint8 *)((uintptr_t)this + g_pEntityManagerProvider->GetGameDataStorage().GetEntityKeyValues().GetContainerTypeOffset());
}

void EntityManager::CEntityKeyValuesProvider::Release()
{
	g_pGameEntitySystem->ReleaseKeyValues(this);
}

CEntityKeyValuesAttribute *EntityManager::CEntityKeyValuesProvider::GetAttribute(const EntityKeyId_t &key, char *psValue) const
{
	return (g_pEntityManagerProvider->GetGameDataStorage().GetEntityKeyValues().GetAttributeFunction())(this, key, psValue);
}

const char *EntityManager::CEntityKeyValuesAttributeProvider::GetValueString(const char *pszDefaultValue) const
{
	return (g_pEntityManagerProvider->GetGameDataStorage().GetEntityKeyValues().AttributeGetValueStringFunction())(this, pszDefaultValue);
}

void EntityManager::CEntityKeyValuesProvider::SetAttributeValue(CEntityKeyValuesAttribute *pAttr, const char *pString)
{
	(g_pEntityManagerProvider->GetGameDataStorage().GetEntityKeyValues().SetAttributeValueFunction())(this, pAttr, pString);
}
