#ifndef ENTITYKEYVALUES_H
#define ENTITYKEYVALUES_H
#ifdef _WIN32
#pragma once
#endif

#include "entity2/entitysystem.h"
#include "tier1/keyvalues3.h"
#include "tier1/utlleanvector.h"

enum EntityKVContextType_t : uint8
{
	ENTITY_KV_CTX_NORMAL = 0,
	ENTITY_KV_CTX_ENTSYSTEM1,
	ENTITY_KV_CTX_ENTSYSTEM2,
	ENTITY_KV_CTX_CUSTOM,
};

enum EntityIOTargetType_t
{
	ENTITY_IO_TARGET_INVALID = -1,
	ENTITY_IO_TARGET_CLASSNAME = 0,
	ENTITY_IO_TARGET_CLASSNAME_DERIVES_FROM = 1,
	ENTITY_IO_TARGET_ENTITYNAME = 2,
	ENTITY_IO_TARGET_CONTAINS_COMPONENT = 3,
	ENTITY_IO_TARGET_SPECIAL_ACTIVATOR = 4,
	ENTITY_IO_TARGET_SPECIAL_CALLER = 5,
	ENTITY_IO_TARGET_EHANDLE = 6,
	ENTITY_IO_TARGET_ENTITYNAME_OR_CLASSNAME = 7,
};

typedef CKV3MemberName EntityKeyId_t;

struct EntityIOConnectionDescFat_t
{ 
	const char* m_pszOutputName;
	EntityIOTargetType_t m_targetType;
	const char* m_pszTargetName;
	const char* m_pszInputName;
	const char* m_pszOverrideParam;
	float m_flDelay;
	int32 m_nTimesToFire;
};

abstract_class IEntityKeyComplex
{
public:
	virtual void DeleteThis() = 0;
};

template < class T >
class CEntityKeyComplex : public IEntityKeyComplex
{
public:
	CEntityKeyComplex( const T& obj ) : m_nRefCount( 0 ), m_Object( obj ) {}
	~CEntityKeyComplex() {}

	virtual void DeleteThis() { delete this; }

	int32 AddRef() { return ++m_nRefCount; }
	int32 Release() 
	{ 
		int32 nRefCount = --m_nRefCount;
		if ( nRefCount == 0 )
			DeleteThis();
		return nRefCount;
	}

public:
	int32 m_nRefCount;
	T m_Object;
};

typedef CEntityKeyComplex<empty_t> CEntityKeyComplexBase;

class CEntityKeyValues
{
public:
	CEntityKeyValues( CKeyValues3Context* kv3ctx = NULL, EntityKVContextType_t kv3ctx_type = ENTITY_KV_CTX_NORMAL );
	~CEntityKeyValues();

	CKeyValues3Context* GetKV3Context();
	void SetKV3Context( CKeyValues3Context* kv3ctx );

	int16 AddRef();
	int16 Release();

	KeyValues3* GetValue( const EntityKeyId_t &key, bool* pIsAttribute = NULL );
	KeyValues3* SetValue( const EntityKeyId_t &key, const char* pAttributeName = NULL );

	void AddConnectionDesc( 
		const char* pszOutputName,	
		EntityIOTargetType_t targetType,
		const char* pszTargetName,
		const char* pszInputName,
		const char* pszOverrideParam,
		float flDelay,
		int32 nTimesToFire );

	void CopyFrom( const CEntityKeyValues* pSrc, bool bRemoveAllKeys = false, bool bSkipEHandles = false );

	void RemoveAllKeys();

private:
	void ReleaseAllComplexKeys();
	void ValidateKV3Context();

private:
	struct EntityComplexKeyListElem_t 
	{
		EntityComplexKeyListElem_t( CEntityKeyComplexBase* pKey, EntityComplexKeyListElem_t* pNext ) : m_pKey( pKey ), m_pNext( pNext ) {}
		~EntityComplexKeyListElem_t() {}

        CEntityKeyComplexBase* m_pKey;
        EntityComplexKeyListElem_t* m_pNext;
    };

	CKeyValues3Context* m_pKV3Context;
	EntityComplexKeyListElem_t* m_pComplexKeys;
	KeyValues3*	m_pKeyValues;
	KeyValues3* m_pAttibutes;
	int16 m_nRefCount;
	int16 m_nQueuedForSpawnCount;
	bool m_bAllowLogging;
	EntityKVContextType_t m_eKV3ContextType;
	CUtlLeanVector<EntityIOConnectionDescFat_t> m_connectionDescs;
};

#endif // ENTITYKEYVALUES_H
