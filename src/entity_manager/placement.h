#include <stddef.h>
#include <stdint.h>

#include <eiface.h>
#include <entity2/entityidentity.h>
#include <entity2/entitysystem.h>
#include <tier0/platform.h>

typedef uint32_t SpawnGroupHandle_t;

enum EntityNetworkingMode_t
{
	ENTITY_NETWORKING_MODE_DEFAULT = 0,
	ENTITY_NETWORKING_MODE_NETWORKED,
	ENTITY_NETWORKING_MODE_NOT_NETWORKED,
};

struct V_uuid_t
{
	uint32 Data1;
	uint16 Data2;
	uint16 Data3;
	uint8 Data4[8];
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
		CBaseEntity *CreateEntity(const char *pszClassname, CEntityIndex iForceEdictIndex = CEntityIndex(-1));

	protected:
		bool LoadGameData(char *psError = NULL, size_t nMaxLength = 0);

	private:
		class CEntitySystemCaller : public CEntitySystem
		{
		public:
			typedef CEntityInstance *(CEntitySystem::*CreateEntity)(SpawnGroupHandle_t hSpawnGroup, const char *pszNameOrDesignName, EntityNetworkingMode_t eNetworkMode, CEntityIndex iForcedIndex, int iForcedSerial, const V_uuid_t *pForcedId, bool bCreateInIsolatedPrecacheList);
		};

		CEntitySystemCaller::CreateEntity m_pfnEntitySystemCreateEntity;
	};
};
