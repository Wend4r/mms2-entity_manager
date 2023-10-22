#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYKEYVALUES_PROVIDER_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYKEYVALUES_PROVIDER_H_

#include <eiface.h>
#include <entity2/entityidentity.h>
#include <entity2/entitysystem.h>
#include <tier1/utlstring.h>

#define ENTITY_KEY_MAGIC_MEOW 0x31415926u

class CEntityKeyValues
{
};

class CEntityKeyValuesAttribute
{
};

class EntityKey
{
public:
	uint32 m_nHashCode;
	const char *m_pszName;
};

namespace EntityManager
{
	class CEntityKeyValuesProvider : public CEntityKeyValues
	{
	public:
		CEntityKeyValuesProvider(void *pUnkEntitySystemSubobject = nullptr, char eUnknownType = 0);
		static CEntityKeyValues *Create(void *pUnkEntitySystemSubobject = nullptr, char eUnknownType = 0);

		CEntityKeyValuesAttribute *GetAttribute(const EntityKey &key, char *psValue = nullptr);
		void SetAttributeValue(CEntityKeyValuesAttribute *pAttr, const char *pString);
	};

	class CEntityKeyValuesAttributeProvider : public CEntityKeyValuesAttribute
	{
	public:
		const char *GetValueString(const char *pszDefaultValue = "");
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYKEYVALUES_PROVIDER_H_
