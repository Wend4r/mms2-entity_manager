#include "entity2/entitykeyvalues.h"
#include "logging.h"

extern CEntitySystem *g_pEntitySystem;

CEntityKeyValues::CEntityKeyValues( CKeyValues3Context* kv3ctx, EntityKVContextType_t kv3ctx_type ) :
	m_pComplexKeys( NULL ),
	m_nRefCount( 0 ),
	m_nQueuedForSpawnCount( 0 ),
	m_bAllowLogging( false ),
	m_eKV3ContextType( kv3ctx_type )
{
	if ( kv3ctx )
	{
		m_pKV3Context = kv3ctx;

		// if ( g_pEntitySystem && kv3ctx == g_pEntitySystem->GetKV3Context() )
		// 	g_pEntitySystem->AddKV3ContextRef();

		m_pKeyValues = m_pKV3Context->AllocKV();
		m_pAttibutes = m_pKV3Context->AllocKV();
	}
	else
	{
		if ( !g_pEntitySystem && ( m_eKV3ContextType == ENTITY_KV_CTX_ENTSYSTEM1 || m_eKV3ContextType == ENTITY_KV_CTX_ENTSYSTEM2 ) )
			m_eKV3ContextType = ENTITY_KV_CTX_NORMAL;

		m_pKV3Context = NULL;
	}
}

CEntityKeyValues::~CEntityKeyValues()
{
	ReleaseAllComplexKeys();

	if ( m_pKV3Context )
	{
		if ( m_eKV3ContextType != ENTITY_KV_CTX_NORMAL )
		{
			m_pKV3Context->FreeKV( m_pKeyValues );
			m_pKV3Context->FreeKV( m_pAttibutes );

			// if ( g_pEntitySystem && m_pKV3Context == g_pEntitySystem->GetKV3Context() )
			// 	g_pEntitySystem->ReleaseKV3ContextRef();
		}
		else
		{
			delete m_pKV3Context;
		}
	}
}

void CEntityKeyValues::ValidateKV3Context()
{
	if ( !m_pKV3Context )
	{
		if ( m_eKV3ContextType == ENTITY_KV_CTX_ENTSYSTEM1 || m_eKV3ContextType == ENTITY_KV_CTX_ENTSYSTEM2 )
		{
			// SetKV3Context( g_pEntitySystem->GetKV3Context() );
		}
		else
		{
			m_pKV3Context = new CKeyValues3Context( true );
		}

		m_pKeyValues = m_pKV3Context->AllocKV();
		m_pAttibutes = m_pKV3Context->AllocKV();
	}
}

CKeyValues3Context* CEntityKeyValues::GetKV3Context()
{
	ValidateKV3Context();
	return m_pKV3Context;
}

void CEntityKeyValues::SetKV3Context( CKeyValues3Context* kv3ctx )
{
	m_pKV3Context = kv3ctx;

// 	if ( kv3ctx == g_pEntitySystem->GetKV3Context() )
// 		g_pEntitySystem->AddKV3ContextRef();
}

int16 CEntityKeyValues::AddRef()
{
	++m_nRefCount;

	if ( m_bAllowLogging )
		InternalMsg( LOG_GENERAL, LS_MESSAGE, "kv 0x%p AddRef refcount == %d\n", this, m_nRefCount );

	return m_nRefCount;
}

int16 CEntityKeyValues::Release()
{
	int16 nRefCount = --m_nRefCount;

	if ( m_bAllowLogging )
		InternalMsg( LOG_GENERAL, LS_MESSAGE, "kv 0x%p Release refcount == %d\n", this, m_nRefCount );

	if ( nRefCount <= 0 )
		delete this;

	return nRefCount;
}

KeyValues3* CEntityKeyValues::GetValue( const EntityKeyId_t &key, bool* pIsAttribute )
{
	if ( !m_pKV3Context )
		return NULL;

	KeyValues3* value = m_pKeyValues->FindOrCreateMember( key );

	if ( value )
	{
		if ( pIsAttribute )
			*pIsAttribute = false;
	}
	else
	{
		value = m_pAttibutes->FindOrCreateMember( key );

		if ( value )
		{
			if ( pIsAttribute )
				*pIsAttribute = true;
		}
	}

	return value;
}

KeyValues3* CEntityKeyValues::SetValue( const EntityKeyId_t &key, const char* pAttributeName )
{
	if ( m_nQueuedForSpawnCount > 0 )
		return NULL;

	ValidateKV3Context();

	bool bIsAttribute;
	KeyValues3* value = GetValue( key, &bIsAttribute );

	if ( value )
	{
		if ( !bIsAttribute && pAttributeName )
		{
			value = NULL;
			Warning( "Attempted to set non-attribute value %s as if it was an attribute!\n", pAttributeName );
		}
		else if ( bIsAttribute )
		{
			pAttributeName = "<none>";

			for ( int i = 0; i < m_pAttibutes->GetMemberCount(); ++i )
			{
				if ( m_pAttibutes->GetMember( i ) != value )
					continue;

				pAttributeName = m_pAttibutes->GetMemberName( i );
				break;
			}

			value = NULL;
			Warning( "Attempted to set attribute %s as if it was a non-attribute key!\n", pAttributeName );
		}
	}
	else
	{
		if ( pAttributeName )
			value = m_pAttibutes->FindOrCreateMember( key );
		else
			value = m_pKeyValues->FindOrCreateMember( key );
	}

	return value;
}

void CEntityKeyValues::AddConnectionDesc( 
	const char* pszOutputName,
	EntityIOTargetType_t targetType,
	const char* pszTargetName,
	const char* pszInputName,
	const char* pszOverrideParam,
	float flDelay,
	int32 nTimesToFire )
{
	if ( m_nQueuedForSpawnCount > 0 )
		return;

	ValidateKV3Context();

	EntityIOConnectionDescFat_t* desc = m_connectionDescs.AddToTailGetPtr();

	desc->m_pszOutputName = m_pKV3Context->AllocString( pszOutputName ? pszOutputName : "" );
	desc->m_targetType = targetType;
	desc->m_pszTargetName = m_pKV3Context->AllocString( pszTargetName ? pszTargetName : "" );
	desc->m_pszInputName = m_pKV3Context->AllocString( pszInputName ? pszInputName : "" );
	desc->m_pszOverrideParam = m_pKV3Context->AllocString( pszOverrideParam ? pszOverrideParam : "" );
	desc->m_flDelay = flDelay;
	desc->m_nTimesToFire = nTimesToFire;
}

void CEntityKeyValues::CopyFrom( const CEntityKeyValues* pSrc, bool bRemoveAllKeys, bool bSkipEHandles )
{
	if ( bRemoveAllKeys )
		RemoveAllKeys();

	for ( EntityComplexKeyListElem_t* pListElem = pSrc->m_pComplexKeys; pListElem != NULL; pListElem = pListElem->m_pNext )
	{
		m_pComplexKeys = new EntityComplexKeyListElem_t( pListElem->m_pKey, m_pComplexKeys->m_pNext );
		pListElem->m_pKey->AddRef();
	}

	KeyValues3* pSrcKV = NULL;

	if ( pSrc->m_pKV3Context )
	{
		if ( pSrc->m_pKeyValues->GetMemberCount() > 0 )
			pSrcKV = pSrc->m_pKeyValues;
		else
			pSrcKV = pSrc->m_pAttibutes;
	}

	while ( pSrcKV )
	{
		for ( int i = 0; i < pSrcKV->GetMemberCount(); ++i )
		{
			const char* pAttributeName = NULL;

			if ( pSrcKV == pSrc->m_pAttibutes )
			{
				pAttributeName = pSrcKV->GetMemberName( i );
			}
			else
			{
				if ( bSkipEHandles && pSrcKV->GetMember( i )->GetSubType() == KV3_SUBTYPE_EHANDLE )
					continue;
			}

			KeyValues3* value = SetValue( pSrcKV->GetMemberNameEx( i ), pAttributeName );

			if ( value )
				*value = *pSrcKV->GetMember( i );
		}

		if ( pSrcKV == pSrc->m_pKeyValues )
			pSrcKV = pSrc->m_pAttibutes;
		else
			pSrcKV = NULL;
	}

	m_connectionDescs.RemoveAll();
	m_connectionDescs.EnsureCapacity( pSrc->m_connectionDescs.Count() );

	FOR_EACH_LEANVEC( pSrc->m_connectionDescs, iter )
	{
		AddConnectionDesc( 
			pSrc->m_connectionDescs[ iter ].m_pszOutputName,
			pSrc->m_connectionDescs[ iter ].m_targetType,
			pSrc->m_connectionDescs[ iter ].m_pszTargetName,
			pSrc->m_connectionDescs[ iter ].m_pszInputName,
			pSrc->m_connectionDescs[ iter ].m_pszOverrideParam,
			pSrc->m_connectionDescs[ iter ].m_flDelay,
			pSrc->m_connectionDescs[ iter ].m_nTimesToFire );
	}
}

void CEntityKeyValues::RemoveAllKeys()
{
	if ( m_nQueuedForSpawnCount > 0 )
		return;

	ReleaseAllComplexKeys();

	if ( m_pKV3Context )
	{
		if ( m_eKV3ContextType != ENTITY_KV_CTX_NORMAL )
		{
			m_pKeyValues->SetToEmptyTable();
			m_pAttibutes->SetToEmptyTable();
		}
		else
		{
			m_pKV3Context->Clear();
			m_pKV3Context->FreeKV( m_pKeyValues );
			m_pKV3Context->FreeKV( m_pAttibutes );
		}
	}
}

void CEntityKeyValues::ReleaseAllComplexKeys()
{
	for ( EntityComplexKeyListElem_t* pListElem = m_pComplexKeys; pListElem != NULL; pListElem = pListElem->m_pNext )
	{
		pListElem->m_pKey->Release();
		delete pListElem;
	}

	m_pComplexKeys = NULL;
}
