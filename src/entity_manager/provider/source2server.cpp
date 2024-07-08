#include "source2server.hpp"
#include <provider.hpp>

#include <dynlibutils/virtual.hpp>

class CGameEventManager;

DLL_IMPORT EntityManager::Provider *g_pEntityManagerProvider;

IGameEventManager2 *EntityManager::CSource2ServerProvider::GetGameEventManager()
{
	CGameEventManager *pEventManager = ((DynLibUtils::VirtualTable *)this)->CallMethod<CGameEventManager *>(g_pEntityManagerProvider->GetGameDataStorage().GetSource2Server().GetGameEventManagerOffset());

	if(pEventManager)
	{
		return (IGameEventManager2 *)((uintptr_t)pEventManager - sizeof(void *));
	}

	return NULL;
}
