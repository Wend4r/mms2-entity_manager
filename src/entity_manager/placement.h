#include <stddef.h>
#include <stdint.h>

#include <eiface.h>
#include <entity2/entityidentity.h>
#include <entity2/entitysystem.h>
#include <tier0/platform.h>

enum EntityNetworkingMode_t
{
	ENTITY_NETWORKING_MODE_DEFAULT = 0,
	ENTITY_NETWORKING_MODE_NETWORKED,
	ENTITY_NETWORKING_MODE_NOT_NETWORKED,
};

namespace EntityManagerSpace
{
	class Placement
	{
	public:
		bool Init(char *psError, size_t nMaxLength);
		bool Load(char *psError = NULL, size_t nMaxLength = 0);
		void Destroy();

	public:
		CBaseEntity *CreateEntity(const char *pszClassName, CEntityIndex iForceEdictIndex = CEntityIndex(-1));

	protected:
		bool LoadGameData(char *psError = NULL, size_t nMaxLength = 0);

	private:
		typedef CEntityInstance *(*CEntitySystem__CreateEntity)(CEntitySystem * const pThis, CEntityIndex iForcedIndex, const char *pszNameOrDesignName, EntityNetworkingMode_t eNetworkMode, SpawnGroupHandle_t hSpawnGroup, int iForcedSerial, bool bCreateInIsolatedPrecacheList);

		CEntitySystem__CreateEntity m_pfnEntitySystemCreateEntity;
	};
};
