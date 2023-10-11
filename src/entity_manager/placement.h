#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PLACEMENT_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PLACEMENT_H_

#include <stddef.h>
#include <stdint.h>

#include <tier0/platform.h>

#include "placement/entitysystem_provider.h"

namespace EntityManagerSpace
{
	class Placement
	{
	public:
		friend class CEntitySystemProvider;

		bool Init(char *psError, size_t nMaxLength);
		bool Load(char *psError = NULL, size_t nMaxLength = 0);
		void Destroy();

	protected:
		bool LoadGameData(char *psError = NULL, size_t nMaxLength = 0);

	private:
		class GameData
		{
		public:
			class EntitySystem
			{
			public:
				typedef CEntityInstance *(*CreateEntityFuncType)(CEntitySystem * const pThis, CEntityIndex iForcedIndex, const char *pszNameOrDesignName, EntityNetworkingMode_t eNetworkMode, SpawnGroupHandle_t hSpawnGroup, int iForcedSerial, bool bCreateInIsolatedPrecacheList);
				CreateEntityFuncType m_pfnCreateEntity;
			};

			EntitySystem m_aEntitySystem;
		};

		GameData m_aData;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PLACEMENT_H_
