#include <entity_manager/provider_agent.hpp>
#include <entity_manager/provider.hpp>

#include <dynlibutils/virtual.hpp>

extern EntityManager::ProviderAgent *g_pEntityManagerProviderAgent;

EntityManager::Provider::GameDataStorage::EntityResourceManifest::EntityResourceManifest()
{
	{
		auto &aCallbacks = m_aOffsetCallbacks;

		aCallbacks.Insert(m_aGameConfig.GetSymbol("CEntityResourceManifest::AddResource"), {[this](const CUtlSymbolLarge &, const ptrdiff_t &nOffset)
		{
			m_nAddResourceIndex = nOffset;
		}});

		m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::EntityResourceManifest::Load(IGameData *pRoot, KeyValues3 *pGameConfig, GameData::CStringVector &vecMessages)
{
	return m_aGameConfig.Load(pRoot, pGameConfig, vecMessages);
}

void EntityManager::Provider::GameDataStorage::EntityResourceManifest::Reset()
{
	m_nAddResourceIndex = -1;
}

void EntityManager::Provider::GameDataStorage::EntityResourceManifest::AddResource(IEntityResourceManifest *pManifest, const char *pszPath) const
{
	reinterpret_cast<DynLibUtils::VirtualTable *>(pManifest)->CallMethod<void, const char *>(m_nAddResourceIndex, pszPath);
}
