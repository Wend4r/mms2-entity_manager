#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYKEYVALUES_PROVIDER_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYKEYVALUES_PROVIDER_H_

#include <eiface.h>
#include <entity2/entityidentity.h>
#include <entity2/entitysystem.h>
#include <tier1/utlstring.h>

class CEntityKeyValues
{
};

struct EntityKey
{
	uint32 m_nHashCode;
	const char *pszName;
};

namespace EntityManagerSpace
{
	class CEntityKeyValuesProvider : public CEntityKeyValues
	{
	public:
		CEntityKeyValuesProvider(void *pUnkEntitySystemSubobject = nullptr, char eUnknownType = 0);
		static CEntityKeyValues *Create(void *pUnkEntitySystemSubobject = nullptr, char eUnknownType = 0);

		void *GetAttribute(const EntityKey &key, char *psValue = nullptr);
		void SetAttributeValue(void *pAttr, const char *pString);
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYKEYVALUES_PROVIDER_H_