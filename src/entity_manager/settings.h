#include <stddef.h>

class KeyValues;

#include "placement.h"

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
		bool InternalLoad(const KeyValues *pEntities, char *psError = NULL, size_t nMaxLength = 0);

	private:
		Placement m_aPlacement;

		KeyValues *m_pEntities;
	};
};
