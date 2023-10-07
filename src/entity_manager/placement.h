#include <stddef.h>

namespace EntityManagerSpace
{
	class Placement
	{
	public:
		bool Init(char *pszError, size_t nMaxLength);
		void SpawnEntity(const char *pszClassname);
		void Destroy();
	};
};
