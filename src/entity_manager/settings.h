#include <stddef.h>

class KeyValues;

namespace EntityManagerSpace
{
	class Settings
	{
	public:
		bool Init(char *pszError, size_t nMaxLength);
		bool Load(const char *pszBasePath, const char *pszMapName, char *psError = NULL, size_t nMaxLength = 0);
		void Clear();
		void Destroy();

	private:
		KeyValues *m_pEntities;
	};
};
