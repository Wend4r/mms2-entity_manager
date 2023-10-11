#include "entitykeyvalues_provider.h"

#include <entity_manager/placement.h>

extern EntityManagerSpace::Placement *g_pEntityManagerPlacement;

EntityManagerSpace::CEntityKeyValuesProvider::CEntityKeyValuesProvider(void *pEntitySystemSubobject, char eSubobjectType)
{
	g_pEntityManagerPlacement->m_aData.m_aEntityKeyValues.m_pfnEntityKeyValues(this, pEntitySystemSubobject, eSubobjectType);
}

CEntityKeyValues *EntityManagerSpace::CEntityKeyValuesProvider::Create(void *pEntitySystemSubobject, char eSubobjectType)
{
	CEntityKeyValues *pNewKeyValues = (CEntityKeyValues *)malloc(g_pEntityManagerPlacement->m_aData.m_aEntityKeyValues.m_nSizeof);

	g_pEntityManagerPlacement->m_aData.m_aEntityKeyValues.m_pfnEntityKeyValues(pNewKeyValues, pEntitySystemSubobject, eSubobjectType);

	return pNewKeyValues;
}

void *EntityManagerSpace::CEntityKeyValuesProvider::GetAttribute(const EntityKey &key, char *psValue)
{
	return g_pEntityManagerPlacement->m_aData.m_aEntityKeyValues.m_pfnGetAttribute(this, key, psValue);
}

void EntityManagerSpace::CEntityKeyValuesProvider::SetAttributeValue(void *pAttr, const char *pString)
{
	g_pEntityManagerPlacement->m_aData.m_aEntityKeyValues.m_pfnSetAttributeValue(this, pAttr, pString);
}
