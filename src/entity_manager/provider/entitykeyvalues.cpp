#include "entitykeyvalues.h"

#include <entity_manager/provider.h>

extern EntityManager::Provider *g_pEntityManagerProvider;

EntityManager::CEntityKeyValuesProvider::CEntityKeyValuesProvider(void *pEntitySystemSubobject, char eSubobjectType)
{
	g_pEntityManagerProvider->m_aData.m_aEntityKeyValues.m_pfnEntityKeyValues(this, pEntitySystemSubobject, eSubobjectType);
}

CEntityKeyValues *EntityManager::CEntityKeyValuesProvider::Create(void *pEntitySystemSubobject, char eSubobjectType)
{
	CEntityKeyValues *pNewKeyValues = (CEntityKeyValues *)malloc(g_pEntityManagerProvider->m_aData.m_aEntityKeyValues.m_nSizeof);

	g_pEntityManagerProvider->m_aData.m_aEntityKeyValues.m_pfnEntityKeyValues(pNewKeyValues, pEntitySystemSubobject, eSubobjectType);

	return pNewKeyValues;
}

CEntityKeyValuesAttribute *EntityManager::CEntityKeyValuesProvider::GetAttribute(const EntityKey &key, char *psValue)
{
	return g_pEntityManagerProvider->m_aData.m_aEntityKeyValues.m_pfnGetAttribute(this, key, psValue);
}

const char *EntityManager::CEntityKeyValuesAttributeProvider::GetValueString(const char *pszDefaultValue)
{
	return g_pEntityManagerProvider->m_aData.m_aEntityKeyValues.m_pfnGetValueString(this, pszDefaultValue);
}

void EntityManager::CEntityKeyValuesProvider::SetAttributeValue(CEntityKeyValuesAttribute *pAttr, const char *pString)
{
	g_pEntityManagerProvider->m_aData.m_aEntityKeyValues.m_pfnSetAttributeValue(this, pAttr, pString);
}
