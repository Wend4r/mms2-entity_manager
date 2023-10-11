#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_SETTINGS_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_SETTINGS_H_

#include <stddef.h>

class KeyValues;

namespace EntityManagerSpace
{
	class Settings
	{
	public:
		bool Init(char *psError, size_t nMaxLength);
		bool Load(const char *pszBasePath, const char *pszMapName, char *psError = NULL, size_t nMaxLength = 0);
		void Clear();
		void Destroy();

	protected:
		bool LoadWorld(const char *pszBaseConfigsDir, char *psError = NULL, size_t nMaxLength = 0);
		bool LoadWorldEntity(KeyValues *pEntityValues, CBaseEntity *&pResultEntity, CEntityKeyValues *&pResultKeyValues, char *psError = NULL, size_t nMaxLength = 0);

	protected:

	private:
		KeyValues *m_pWorld;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_SETTINGS_H_
