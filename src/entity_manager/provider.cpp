#include "provider.h"

#include "gamedata.h"
#include "provider_agent.h"

#include <entity2/entitysystem.h>

#define GAMECONFIG_FOLDER_DIR "gamedata"
#define GAMECONFIG_ENTITYKEYVALUES_FILENAME "entitykeyvalues.games.txt"
#define GAMECONFIG_ENTITYSYSTEM_FILENAME "entitysystem.games.txt"
#define GAMECONFIG_SPAWNGROUP_FILENAME "spawngroup.games.txt"

extern EntityManager::ProviderAgent *g_pEntityManagerProviderAgent;

extern IFileSystem *filesystem;

bool EntityManager::Provider::Init(char *psError, size_t nMaxLength)
{
	char sGameDataError[256];

	bool bResult = this->m_aData.Init((char *)sGameDataError, sizeof(sGameDataError));

	if(!bResult && psError)
	{
		snprintf(psError, nMaxLength, "Failed to init a gamedata: %s", sGameDataError);
	}

	return bResult;
}

bool EntityManager::Provider::Load(const char *pszBaseDir, char *psError, size_t nMaxLength)
{
	bool bResult = this->LoadGameData(pszBaseDir, psError, nMaxLength);

	if(bResult)
	{
		// Next load ...
	}

	return bResult;
}

void EntityManager::Provider::Destroy()
{
	// ...
}

bool EntityManager::Provider::LoadGameData(const char *pszBaseDir, char *psError, size_t nMaxLength)
{
	char sBaseConfigDir[MAX_PATH];

	snprintf((char *)sBaseConfigDir, sizeof(sBaseConfigDir), 
#ifdef PLATFORM_WINDOWS
		"%s\\%s", 
#else
		"%s/%s", 
#endif
		pszBaseDir, GAMECONFIG_FOLDER_DIR);

	char sGameDataError[256];

	bool bResult = this->m_aStorage.Load((IGameData *)&this->m_aData, (const char *)sBaseConfigDir, (char *)sGameDataError, sizeof(sGameDataError));

	if(!bResult && psError)
	{
		snprintf(psError, nMaxLength, "Failed to load a gamedata: %s", sGameDataError);
	}

	return bResult;
}

bool EntityManager::Provider::GameDataStorage::Load(IGameData *pRoot, const char *pszBaseConfigDir, char *psError, size_t nMaxLength)
{
	KeyValues *pGameConfig = new KeyValues("Games");

	char sConfigFile[MAX_PATH];

	bool bResult = true;

	struct
	{
		const char *pszFilename;
		bool (EntityManager::Provider::GameDataStorage::*pfnLoad)(IGameData *, KeyValues *, char *, size_t);
	} aConfigs[] =
	{
		{
			GAMECONFIG_ENTITYKEYVALUES_FILENAME,
			&GameDataStorage::LoadEntityKeyValues
		},
		{
			GAMECONFIG_ENTITYSYSTEM_FILENAME,
			&GameDataStorage::LoadEntitySystem
		},
		{
			GAMECONFIG_SPAWNGROUP_FILENAME,
			&GameDataStorage::LoadEntitySpawnGroup
		}
	};

	for(size_t n = 0, nSize = std::size(aConfigs); n < nSize; n++)
	{
		snprintf((char *)sConfigFile, sizeof(sConfigFile), 
#ifdef PLATFORM_WINDOWS
			"%s\\%s",
#else
			"%s/%s", 
#endif
			pszBaseConfigDir, aConfigs[n].pszFilename);

		bResult = pGameConfig->LoadFromFile(filesystem, (const char *)sConfigFile);

		if(bResult)
		{
			bResult = (this->*(aConfigs[n].pfnLoad))(pRoot, pGameConfig, psError, nMaxLength);

			if(bResult)
			{
				pGameConfig->Clear();
			}
			else
			{
				break;
			}
		}
		else if(psError)
		{
			snprintf(psError, nMaxLength, "Failed to load KeyValues from \"%s\" file", sConfigFile);

			break;
		}
	}

	delete pGameConfig;

	return bResult;
}

bool EntityManager::Provider::GameDataStorage::LoadEntityKeyValues(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aEntityKeyValues.Load(pRoot, pGameConfig, psError, nMaxLength);
}

bool EntityManager::Provider::GameDataStorage::LoadEntitySystem(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aEntitySystem.Load(pRoot, pGameConfig, psError, nMaxLength);
}

bool EntityManager::Provider::GameDataStorage::LoadEntitySpawnGroup(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aSpawnGroup.Load(pRoot, pGameConfig, psError, nMaxLength);
}

EntityManager::Provider::GameDataStorage::EntityKeyValues::EntityKeyValues()
{
	{
		auto &aCallbacks = this->m_aAddressCallbacks;

		aCallbacks.Insert("CEntityKeyValues::CEntityKeyValues", [this](const std::string &, const CMemory &pFucntion)
		{
			this->m_pfnEntityKeyValues = pFucntion.RCast<decltype(this->m_pfnEntityKeyValues)>();
		});
		aCallbacks.Insert("CEntityKeyValues::GetAttribute", [this](const std::string &, const CMemory &pFucntion)
		{
			this->m_pfnGetAttribute = pFucntion.RCast<decltype(this->m_pfnGetAttribute)>();
		});
		aCallbacks.Insert("CEntityKeyValuesAttribute::GetValueString", [this](const std::string &, const CMemory &pFucntion)
		{
			this->m_pfnAttributeGetValueString = pFucntion.RCast<decltype(this->m_pfnAttributeGetValueString)>();
		});
		aCallbacks.Insert("CEntityKeyValues::SetAttributeValue", [this](const std::string &, const CMemory &pFucntion)
		{
			this->m_pfnSetAttributeValue = pFucntion.RCast<decltype(this->m_pfnSetAttributeValue)>();
		});

		this->m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}
	
	{
		auto &aCallbacks = this->m_aOffsetCallbacks;

		aCallbacks.Insert("sizeof(CEntityKeyValues)", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nSizeof = nOffset;
		});
		aCallbacks.Insert("CEntityKeyValues::m_nRefCount", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nRefCountOffset = nOffset;
		});

		this->m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::EntityKeyValues::Load(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aGameConfig.Load(pRoot, pGameConfig, psError, nMaxLength);
}

void EntityManager::Provider::GameDataStorage::EntityKeyValues::Reset()
{
	this->m_aGameConfig.ClearValues();

	this->m_pfnEntityKeyValues = nullptr;
	this->m_pfnGetAttribute = nullptr;
	this->m_pfnAttributeGetValueString = nullptr;
	this->m_pfnSetAttributeValue = nullptr;

	this->m_nSizeof = -1;
	this->m_nRefCountOffset = -1;
}



EntityManager::Provider::GameDataStorage::EntityKeyValues::OnEntityKeyValuesPtr EntityManager::Provider::GameDataStorage::EntityKeyValues::EntityKeyValuesFunction() const
{
	return this->m_pfnEntityKeyValues;
}

EntityManager::Provider::GameDataStorage::EntityKeyValues::OnGetAttributePtr EntityManager::Provider::GameDataStorage::EntityKeyValues::GetAttributeFunction() const
{
	return this->m_pfnGetAttribute;
}

EntityManager::Provider::GameDataStorage::EntityKeyValues::OnAttributeGetValueStringPtr EntityManager::Provider::GameDataStorage::EntityKeyValues::AttributeGetValueStringFunction() const
{
	return this->m_pfnAttributeGetValueString;
}

EntityManager::Provider::GameDataStorage::EntityKeyValues::OnSetAttributeValuePtr EntityManager::Provider::GameDataStorage::EntityKeyValues::SetAttributeValueFunction() const
{
	return this->m_pfnSetAttributeValue;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::EntityKeyValues::GetSizeof() const
{
	return this->m_nSizeof;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::EntityKeyValues::GetRefCountOffset() const
{
	return this->m_nRefCountOffset;
}

EntityManager::Provider::GameDataStorage::EntitySystem::EntitySystem()
{
	{
		auto &aCallbacks = this->m_aAddressCallbacks;

		aCallbacks.Insert("CEntitySystem::CreateEntity", [this](const std::string &, const CMemory &pFucntion)
		{
			this->m_pfnCreateEntity = pFucntion.RCast<decltype(this->m_pfnCreateEntity)>();
		});
		aCallbacks.Insert("CEntitySystem::QueueSpawnEntity", [this](const std::string &, const CMemory &pFucntion)
		{
			this->m_pfnQueueSpawnEntity = pFucntion.RCast<decltype(this->m_pfnQueueSpawnEntity)>();
		});
		aCallbacks.Insert("CEntitySystem::ExecuteQueuedCreation", [this](const std::string &, const CMemory &pFucntion)
		{
			this->m_pfnExecuteQueuedCreation = pFucntion.RCast<decltype(this->m_pfnExecuteQueuedCreation)>();
		});

		this->m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}
	
	{
		auto &aCallbacks = this->m_aOffsetCallbacks;

		aCallbacks.Insert("CGameResourceService::m_pEntitySystem", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nGameResourceServiceEntitySystemOffset = nOffset;
			g_pEntityManagerProviderAgent->NotifyEntitySystemUpdated();
		});
		aCallbacks.Insert("CEntitySystem::m_pCurrentManifest", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nCurrentManifestOffset = nOffset;
		});
		aCallbacks.Insert("CEntitySystem::m_aKeyValuesMemoryPool", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nKeyValuesMemoryPoolOffset = nOffset;
		});

		this->m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::EntitySystem::Load(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aGameConfig.Load(pRoot, pGameConfig, psError, nMaxLength);
}

void EntityManager::Provider::GameDataStorage::EntitySystem::Reset()
{
	this->m_aGameConfig.ClearValues();

	this->m_pfnCreateEntity = nullptr;
	this->m_pfnQueueSpawnEntity = nullptr;
	this->m_pfnExecuteQueuedCreation = nullptr;

	this->m_nCurrentManifestOffset = -1;
	this->m_nKeyValuesMemoryPoolOffset = -1;
}

EntityManager::Provider::GameDataStorage::EntitySystem::OnCreateEntityPtr EntityManager::Provider::GameDataStorage::EntitySystem::CreateEntityFunction() const
{
	return this->m_pfnCreateEntity;
}

EntityManager::Provider::GameDataStorage::EntitySystem::OnQueueSpawnEntityPtr EntityManager::Provider::GameDataStorage::EntitySystem::QueueSpawnEntityFunction() const
{
	return this->m_pfnQueueSpawnEntity;
}

EntityManager::Provider::GameDataStorage::EntitySystem::OnExecuteQueuedCreationPtr EntityManager::Provider::GameDataStorage::EntitySystem::ExecuteQueuedCreationFunction() const
{
	return this->m_pfnExecuteQueuedCreation;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::EntitySystem::GetGameResourceServiceEntitySystemOffset() const
{
	return this->m_nGameResourceServiceEntitySystemOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::EntitySystem::GetCurrentManifestOffset() const
{
	return this->m_nCurrentManifestOffset;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::EntitySystem::GetKeyValuesMemoryPoolOffset() const
{
	return this->m_nKeyValuesMemoryPoolOffset;
}

EntityManager::Provider::GameDataStorage::SpawnGroup::SpawnGroup()
{
	{
		auto &aCallbacks = this->m_aAddressCallbacks;

		aCallbacks.Insert("&g_pSpawnGroupMgr", [this](const std::string &, const CMemory &pFucntion)
		{
			this->m_ppSpawnGroupMgrAddress = pFucntion.RCast<decltype(this->m_ppSpawnGroupMgrAddress)>();
			g_pEntityManagerProviderAgent->NotifySpawnGroupMgrUpdated();
		});

		this->m_aGameConfig.GetAddresses().AddListener(&aCallbacks);
	}
	
	{
		auto &aCallbacks = this->m_aOffsetCallbacks;

		aCallbacks.Insert("CSpawnGroupMgrGameSystem::m_SpawnGroups", [this](const std::string &, const ptrdiff_t &nOffset)
		{
			this->m_nMgrGameSystemSpawnGroupsOffset = nOffset;
		});

		this->m_aGameConfig.GetOffsets().AddListener(&aCallbacks);
	}
}

bool EntityManager::Provider::GameDataStorage::SpawnGroup::Load(IGameData *pRoot, KeyValues *pGameConfig, char *psError, size_t nMaxLength)
{
	return this->m_aGameConfig.Load(pRoot, pGameConfig, psError, nMaxLength);
}

void EntityManager::Provider::GameDataStorage::SpawnGroup::Reset()
{
	this->m_aGameConfig.ClearValues();

	this->m_ppSpawnGroupMgrAddress = nullptr;

	this->m_nMgrGameSystemSpawnGroupsOffset = -1;
}

CSpawnGroupMgrGameSystem **EntityManager::Provider::GameDataStorage::SpawnGroup::GetSpawnGroupMgrAddress() const
{
	return this->m_ppSpawnGroupMgrAddress;
}

ptrdiff_t EntityManager::Provider::GameDataStorage::SpawnGroup::GetMgrGameSystemSpawnGroupsOffset() const
{
	return this->m_nMgrGameSystemSpawnGroupsOffset;
}



const EntityManager::Provider::GameDataStorage::EntityKeyValues &EntityManager::Provider::GameDataStorage::GetEntityKeyValues() const
{
	return this->m_aEntityKeyValues;
}

const EntityManager::Provider::GameDataStorage::EntitySystem &EntityManager::Provider::GameDataStorage::GetEntitySystem() const
{
	return this->m_aEntitySystem;
}

const EntityManager::Provider::GameDataStorage::SpawnGroup &EntityManager::Provider::GameDataStorage::GetSpawnGroup() const
{
	return this->m_aSpawnGroup;
}

const EntityManager::Provider::GameDataStorage &EntityManager::Provider::GetGameDataStorage() const
{
	return this->m_aStorage;
}
