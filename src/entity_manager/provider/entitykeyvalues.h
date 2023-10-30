#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYKEYVALUES_PROVIDER_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYKEYVALUES_PROVIDER_H_

#include <eiface.h>
#include <tier1/utlstring.h>
#include <tier1/utlscratchmemory.h>
#include <entity2/entityidentity.h>
#include <entity2/entitysystem.h>

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
		static CEntityKeyValues *Create(CUtlScratchMemoryPool *pMemoryPool = nullptr, char eContainerType = 0);

	public: // Reference.
		void AddRef();
		uint16 GetRefCount() const;
		void Release();

	public: // Attributes.
		CEntityKeyValuesAttribute *GetAttribute(const EntityKey &key, char *psValue = nullptr) const;
		void SetAttributeValue(CEntityKeyValuesAttribute *pAttr, const char *pString);
	};

	class CEntityKeyValuesAttributeProvider : public CEntityKeyValuesAttribute
	{
	public:
		const char *GetValueString(const char *pszDefaultValue = "") const;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYKEYVALUES_PROVIDER_H_
