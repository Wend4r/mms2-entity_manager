#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_SOURCE2SERVER_PROVIDER_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_SOURCE2SERVER_PROVIDER_H_

#include <eiface.h>

class IGameEventManager2;

class CSource2Server : public ISource2Server
{
};

namespace EntityManager
{
	class CSource2ServerProvider : public CSource2Server
	{
	public:
		IGameEventManager2 *GetGameEventManager();
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_SOURCE2SERVER_PROVIDER_H_
