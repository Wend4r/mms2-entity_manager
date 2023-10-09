#include "placement.h"
#include "gamedata.h"

#include <entity2/entitysystem.h>

extern CEntitySystem *g_pEntitySystem;

extern EntityManagerSpace::GameData *g_pEntityManagerGameData;

bool EntityManagerSpace::Placement::Init(char *psError, size_t nMaxLength)
{
	return true;
}

void EntityManagerSpace::Placement::SpawnEntity(const char *pszClassname)
{
	// ...
}

void EntityManagerSpace::Placement::Destroy()
{
	// ...
}
