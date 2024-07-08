#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_GAMESYSTEM_HPP_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_GAMESYSTEM_HPP_

#include <igamesystem.h>

class CBaseGameSystemFactory;

class CGameSystem : public IGameSystem
{
};

namespace EntityManager
{
	class CGameSystemProvider : public CGameSystem
	{
	public:
		CBaseGameSystemFactory **GetGameSystemFactoryBase();
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PROVIDER_GAMESYSTEM_HPP_
