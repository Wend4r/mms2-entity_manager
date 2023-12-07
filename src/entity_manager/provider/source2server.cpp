#include "source2server.h"
#include "provider.h"

#include <memory_utils/virtual.h>

class CGameEventManager;

extern EntityManager::Provider *g_pEntityManagerProvider;

IGameEventManager2 *EntityManager::CSource2ServerProvider::GetGameEventManager()
{
	CGameEventManager *pEventManager = ((VirtualTable *)this)->CallMethod<CGameEventManager *>(g_pEntityManagerProvider->GetGameDataStorage().GetSource2Server().GetGameEventManagerOffset());

	if(pEventManager)
	{
		return (IGameEventManager2 *)((uintptr_t)pEventManager - sizeof(void *));
	}

	return NULL;
}
