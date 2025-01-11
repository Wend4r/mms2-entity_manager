#include <entity_manager/provider/gamesystem.hpp>
#include <entity_manager/provider.hpp>

extern EntityManager::Provider *g_pEntityManagerProvider;

CBaseGameSystemFactory **EntityManager::CGameSystemProvider::GetGameSystemFactoryBase()
{
	return g_pEntityManagerProvider->GetGameDataStorage().GetGameSystem().GetBaseGameSystemFactoryFirst();
}
