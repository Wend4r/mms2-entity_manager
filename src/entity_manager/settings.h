#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_SETTINGS_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_SETTINGS_H_

#include <stddef.h>

#include <eiface.h>
#include <entity2/entityidentity.h>

class KeyValues;
class CBaseEntity;
class CEntityKeyValues;

namespace EntityManagerSpace
{
	class Settings
	{
	public:
		bool Init(char *psError, size_t nMaxLength);
		bool Load(SpawnGroupHandle_t hSpawnGroup = (SpawnGroupHandle_t)-1, const char *pszBasePath = "", const char *pszMapName = "NONE", char *psError = NULL, size_t nMaxLength = 0);
		void Clear();
		void Destroy();

	protected:
		bool LoadWorld(SpawnGroupHandle_t hSpawnGroup, const char *pszBaseConfigsDir, char *psError = NULL, size_t nMaxLength = 0);

	protected:

	private:
		KeyValues *m_pWorld;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_SETTINGS_H_
