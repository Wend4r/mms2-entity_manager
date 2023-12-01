#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYKEYVALUES_PROVIDER_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYKEYVALUES_PROVIDER_H_

#include <eiface.h>
// #include <variant.h>
#include <tier0/utlstring.h>
#include <tier0/utlstringtoken.h>
#include <tier0/utlscratchmemory.h>
#include <tier1/keyvalues3.h>
#include <entity2/entityidentity.h>
#include <entity2/entitysystem.h>

class CEntityKeyValues
{
};

class CEntityKeyValuesAttribute
{
};

typedef CKV3MemberName EntityKeyId_t;

namespace EntityManager
{
	class CEntityKeyValuesProvider : public CEntityKeyValues
	{
	public:
		static CEntityKeyValues *Create(CKeyValues3Context *pClusterAllocator = nullptr, char eContextType = 0);

	public:
		KeyValues3 *Root();
		const KeyValues3 *Root() const;

	public: // Reference.
		void AddRef();
		uint16 GetRefCount() const;
		uint8 GetContainerType() const;
		void Release();

	public: // Attributes.
		CEntityKeyValuesAttribute *GetAttribute(const EntityKeyId_t &key, char *psValue = nullptr) const;
		void SetAttributeValue(CEntityKeyValuesAttribute *pAttr, const char *pString);
	};

	class CEntityKeyValuesAttributeProvider : public CEntityKeyValuesAttribute
	{
	public:
		const char *GetValueString(const char *pszDefaultValue = "") const;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_ENTITYKEYVALUES_PROVIDER_H_
