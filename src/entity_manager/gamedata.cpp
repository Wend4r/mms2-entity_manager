#include "gamedata.h"
#include "logger.h"

#include <stdlib.h>

#include <tier1/utlstring.h>

#include <entity2/entitysystem.h>
#include <tier0/dbg.h>
#include <tier0/platform.h>
#include <tier1/KeyValues.h>

#define GAMECONFIG_FOLDER_DIR "gamedata"
#define GAMECONFIG_ENTITY_SYSTEM_FILENAME "entitysystem.games.txt"
#define GAMECONFIG_ENTITY_KEYVALUES_FILENAME "entitykeyvalues.games.txt"
#define GAMECONFIG_SPAWN_GROUP_FILENAME "spawngroup.games.txt"

extern IVEngineServer *engine;
extern IFileSystem *filesystem;
extern IServerGameDLL *server;

extern EntityManager::GameData *g_pEntityManagerGameData;
extern EntityManager::Logger *g_pEntityManagerLogger;

CModule g_aLibEngine, 
        g_aLibServer;

bool EntityManager::GameData::Init(char *psError, size_t nMaxLength)
{
	bool bResult = g_aLibEngine.InitFromMemory(engine);

	if(bResult)
	{
		this->m_aLibraryMap["engine"] = &g_aLibEngine;

		bResult = g_aLibServer.InitFromMemory(server);

		if(bResult)
		{
			this->m_aLibraryMap["server"] = &g_aLibServer;
		}
		else
		{
			strncpy(psError, "Failed to get server module", nMaxLength);
		}
	}
	else
	{
		strncpy(psError, "Failed to get engine module", nMaxLength);
	}

	return true;
}

bool EntityManager::GameData::Load(const char *pszBasePath, char *psError, size_t nMaxLength)
{
	char sBaseGameConfigDir[MAX_PATH];

	snprintf((char *)sBaseGameConfigDir, sizeof(sBaseGameConfigDir), 
#ifdef PLATFORM_WINDOWS
		"%s\\%s", 
#else
		"%s/%s", 
#endif
		pszBasePath, GAMECONFIG_FOLDER_DIR);

	bool bResult = this->LoadEntityKeyValues((const char *)sBaseGameConfigDir, psError, nMaxLength);

	if(bResult)
	{
		bResult = this->LoadEntitySystem((const char *)sBaseGameConfigDir, psError, nMaxLength);

		if(bResult)
		{
			bResult = this->LoadSpawnGroup((const char *)sBaseGameConfigDir, psError, nMaxLength);
		}
	}

	return bResult;
}

void EntityManager::GameData::Clear()
{
	this->m_aEntitySystemConfig.Clear();
}

void EntityManager::GameData::Destroy()
{
	// ...
}

const CModule *EntityManager::GameData::FindLibrary(const char *pszName)
{
	auto itResult = this->m_aLibraryMap.find(pszName);

	return itResult == this->m_aLibraryMap.cend() ? nullptr : itResult->second;
}

bool EntityManager::GameData::LoadEntityKeyValues(const char *pszBaseGameConfigDir, char *psError, size_t nMaxLength)
{
	char sConfigFile[MAX_PATH];

	snprintf((char *)sConfigFile, sizeof(sConfigFile), 
#ifdef PLATFORM_WINDOWS
		"%s\\%s",
#else
		"%s/%s", 
#endif
		pszBaseGameConfigDir, GAMECONFIG_ENTITY_KEYVALUES_FILENAME);

	KeyValues *pGamesValues = new KeyValues("Games");

	bool bResult = pGamesValues->LoadFromFile(filesystem, (const char *)sConfigFile);

	if(bResult)
	{
		char sGameConfigError[1024];

		bResult = this->m_aEntityKeyValuesConfig.Load(pGamesValues, (char *)sGameConfigError, sizeof(sGameConfigError));

		if(!bResult && psError)
		{
			snprintf(psError, nMaxLength, "Failed to load a entity keyvalues: %s", sGameConfigError);
		}
	}
	else if(psError)
	{
		snprintf(psError, nMaxLength, "Can't to load KeyValues from \"%s\" file", sConfigFile);
	}

	delete pGamesValues;

	return bResult;
}

bool EntityManager::GameData::LoadEntitySystem(const char *pszBaseGameConfigDir, char *psError, size_t nMaxLength)
{
	char sConfigFile[MAX_PATH];

	snprintf((char *)sConfigFile, sizeof(sConfigFile), 
#ifdef PLATFORM_WINDOWS
		"%s\\%s",
#else
		"%s/%s", 
#endif
		pszBaseGameConfigDir, GAMECONFIG_ENTITY_SYSTEM_FILENAME);

	KeyValues *pGamesValues = new KeyValues("Games");

	bool bResult = pGamesValues->LoadFromFile(filesystem, (const char *)sConfigFile);

	if(bResult)
	{
		char sGameConfigError[1024];

		bResult = this->m_aEntitySystemConfig.Load(pGamesValues, (char *)sGameConfigError, sizeof(sGameConfigError));

		if(!bResult && psError)
		{
			snprintf(psError, nMaxLength, "Failed to load a entity system: %s", sGameConfigError);
		}
	}
	else if(psError)
	{
		snprintf(psError, nMaxLength, "Can't to load KeyValues from \"%s\" file", sConfigFile);
	}

	delete pGamesValues;

	return bResult;
}

bool EntityManager::GameData::LoadSpawnGroup(const char *pszBaseGameConfigDir, char *psError, size_t nMaxLength)
{
	char sConfigFile[MAX_PATH];

	snprintf((char *)sConfigFile, sizeof(sConfigFile), 
#ifdef PLATFORM_WINDOWS
		"%s\\%s",
#else
		"%s/%s", 
#endif
		pszBaseGameConfigDir, GAMECONFIG_SPAWN_GROUP_FILENAME);

	KeyValues *pGamesValues = new KeyValues("Games");

	bool bResult = pGamesValues->LoadFromFile(filesystem, (const char *)sConfigFile);

	if(bResult)
	{
		char sGameConfigError[1024];

		bResult = this->m_aSpawnGroupConfig.Load(pGamesValues, (char *)sGameConfigError, sizeof(sGameConfigError));

		if(!bResult && psError)
		{
			snprintf(psError, nMaxLength, "Failed to load a spawn group: %s", sGameConfigError);
		}
	}
	else if(psError)
	{
		snprintf(psError, nMaxLength, "Can't to load KeyValues from \"%s\" file", sConfigFile);
	}

	delete pGamesValues;

	return bResult;
}

CMemory EntityManager::GameData::GetEntityKeyValuesAddress(const std::string &sName)
{
	return this->m_aEntityKeyValuesConfig.GetAddress(sName);
}

ptrdiff_t EntityManager::GameData::GetEntityKeyValuesOffset(const std::string &sName)
{
	return this->m_aEntityKeyValuesConfig.GetOffset(sName);
}

CMemory EntityManager::GameData::GetEntitySystemAddress(const std::string &sName)
{
	return this->m_aEntitySystemConfig.GetAddress(sName);
}

ptrdiff_t EntityManager::GameData::GetEntitySystemOffset(const std::string &sName)
{
	return this->m_aEntitySystemConfig.GetOffset(sName);
}

CMemory EntityManager::GameData::GetSpawnGroupAddress(const std::string &sName)
{
	return this->m_aSpawnGroupConfig.GetAddress(sName);
}

ptrdiff_t EntityManager::GameData::GetSpawnGroupOffset(const std::string &sName)
{
	return this->m_aSpawnGroupConfig.GetOffset(sName);
}

const char *EntityManager::GameData::GetSourceEngineName()
{
#if SOURCE_ENGINE == SE_CS2
	return "cs2";
#elif SOURCE_ENGINE == SE_DOTA
	return "dota";
#else
#	error "Unknown engine type"
	return "unknown";
#endif
}

EntityManager::GameData::Platform EntityManager::GameData::GetCurrentPlatform()
{
#if defined(_WINDOWS)
#	if defined(X64BITS)
	return Platform::WINDOWS64;
#	else
	return Platform::WINDOWS;
#	endif
#elif defined(_LINUX)
#	if defined(X64BITS)
	return Platform::LINUX64;
#	else
	return Platform::LINUX;
#	endif
#else
#	error Unsupported platform
	return Platform::UNKNOWN;
#endif
}

const char *EntityManager::GameData::GetCurrentPlatformName()
{
	return ThisClass::GetPlatformName(ThisClass::GetCurrentPlatform());
}

const char *EntityManager::GameData::GetPlatformName(Platform eElm)
{
	const char *sPlatformNames[Platform::PLATFORM_MAX] =
	{
		"windows",
		"windows64",

		"linux",
		"linux64",

		"mac",
		"mac64"
	};

	return sPlatformNames[eElm];
}

ptrdiff_t EntityManager::GameData::ReadOffset(const char *pszValue)
{
	return static_cast<ptrdiff_t>(strtol(pszValue, NULL, 0));
}

bool EntityManager::GameData::Config::Load(KeyValues *pGamesValues, char *psError, size_t nMaxLength)
{
	const char *pszEngineName = EntityManager::GameData::GetSourceEngineName();

	KeyValues *pEngineValues = pGamesValues->FindKey(pszEngineName, false);

	bool bResult = pEngineValues != nullptr;

	if(bResult)
	{
		this->LoadEngine(pEngineValues, psError, nMaxLength);
	}
	else if(psError)
	{
		snprintf(psError, nMaxLength, "Failed to find \"%s\" section", pszEngineName);
	}

	return bResult;
}

bool EntityManager::GameData::Config::LoadEngine(KeyValues *pEngineValues, char *psError, size_t nMaxLength)
{
	KeyValues *pSectionValues = pEngineValues->FindKey("Signatures", false);

	bool bResult = true;

	if(pSectionValues) // Ignore the section not found for result.
	{
		bResult = this->LoadEngineSignatures(pSectionValues, psError, nMaxLength);

		if(bResult)
		{
			pSectionValues = pEngineValues->FindKey("Offsets", false);

			if(pSectionValues) // Same ignore.
			{
				bResult = this->LoadEngineOffsets(pSectionValues, psError, nMaxLength);
			}

			if(bResult)
			{
				pSectionValues = pEngineValues->FindKey("Addresses", false);

				if(pSectionValues)
				{
					bResult = this->LoadEngineAddresses(pSectionValues, psError, nMaxLength);
				}
			}
		}
	}

	return bResult;
}

bool EntityManager::GameData::Config::LoadEngineSignatures(KeyValues *pSignaturesValues, char *psError, size_t nMaxLength)
{
	KeyValues *pSigSection = pSignaturesValues->GetFirstSubKey();

	bool bResult = pSigSection != nullptr;

	if(bResult)
	{
		const char *pszLibraryKey = "library", 
		           *pszPlatformKey = EntityManager::GameData::GetCurrentPlatformName();

		do
		{
			const char *pszSigName = pSigSection->GetName();

			KeyValues *pLibraryValues = pSigSection->FindKey(pszLibraryKey, false);

			bResult = pLibraryValues != nullptr;

			if(bResult)
			{
				const char *pszLibraryName = pLibraryValues->GetString(nullptr, "unknown");

				const CModule *pLibModule = g_pEntityManagerGameData->FindLibrary(pszLibraryName);

				bResult = (bool)pLibModule;

				if(bResult)
				{
					KeyValues *pPlatformValues = pSigSection->FindKey(pszPlatformKey, false);

					bResult = pPlatformValues != nullptr;

					if(pPlatformValues)
					{
						const char *pszSignature = pPlatformValues->GetString(nullptr);

						CMemory pSigResult = pLibModule->FindPatternSIMD(pszSignature);

						bResult = (bool)pSigResult;

						if(bResult)
						{
							this->SetAddress(pszSigName, pSigResult);
						}
						else if(psError)
						{
							snprintf(psError, nMaxLength, "Failed to find \"%s\" signature", pszSigName);
						}
					}
					else if(psError)
					{
						snprintf(psError, nMaxLength, "Failed to get platform (\"%s\" key) at \"%s\" signature", pszPlatformKey, pszSigName);
					}
				}
				else if(psError)
				{
					snprintf(psError, nMaxLength, "Unknown \"%s\" library at \"%s\" signature", pszLibraryName, pszSigName);
				}
			}
			else if(psError)
			{
				snprintf(psError, nMaxLength, "Failed to get library (\"%s\" key) at \"%s\" signature", pszLibraryKey, pszSigName);
			}

			pSigSection = pSigSection->GetNextKey();
		}
		while(pSigSection);
	}
	else if(psError)
	{
		strncpy(psError, "Signatures section is empty", nMaxLength);
	}

	return bResult;
}

bool EntityManager::GameData::Config::LoadEngineOffsets(KeyValues *pOffsetsValues, char *psError, size_t nMaxLength)
{
	KeyValues *pOffsetSection = pOffsetsValues->GetFirstSubKey();

	bool bResult = pOffsetSection != nullptr;

	if(bResult)
	{
		const char *pszPlatformKey = EntityManager::GameData::GetCurrentPlatformName();

		do
		{
			const char *pszOffsetName = pOffsetSection->GetName();

			KeyValues *pPlatformValues = pOffsetSection->FindKey(pszPlatformKey, false);

			bResult = pPlatformValues != nullptr;

			if(pPlatformValues)
			{
				this->SetOffset(pszOffsetName, EntityManager::GameData::ReadOffset(pPlatformValues->GetString(NULL)));
			}
			else if(psError)
			{
				snprintf(psError, nMaxLength, "Failed to get platform (\"%s\" key) at \"%s\" signature", pszPlatformKey, pszOffsetName);
			}

			pOffsetSection = pOffsetSection->GetNextKey();
		}
		while(pOffsetSection);
	}
	else if(psError)
	{
		strncpy(psError, "Offsets section is empty", nMaxLength);
	}

	return bResult;
}

bool EntityManager::GameData::Config::LoadEngineAddresses(KeyValues *pAddressesValues, char *psError, size_t nMaxLength)
{
	KeyValues *pAddrSection = pAddressesValues->GetFirstSubKey();

	bool bResult = pAddrSection != nullptr;

	if(bResult)
	{
		const char *pszSignatureKey = "signature";

		char sAddressActionsError[256];

		do
		{
			const char *pszAddressName = pAddrSection->GetName();

			KeyValues *pSignatureValues = pAddrSection->FindKey(pszSignatureKey, false);

			bResult = pSignatureValues != nullptr;

			if(bResult)
			{
				const char *pszSignatureName = pSignatureValues->GetString(NULL);

				CMemory pSigAddress = this->GetAddress(pszSignatureName);

				if(pSigAddress)
				{
					uintptr_t pAddrCur = pSigAddress.GetPtr();

					// Remove an extra keys.
					{
						pAddrSection->RemoveSubKey(pSignatureValues, true /* bDelete */, true);

						int iCurrentPlat = ThisClass::GetCurrentPlatform();

						for(int iPlat = PLATFORM_FIRST; iPlat < PLATFORM_MAX; iPlat++)
						{
							if(iCurrentPlat != iPlat)
							{
								pAddrSection->FindAndDeleteSubKey(ThisClass::GetPlatformName((ThisClass::Platform)iPlat));
							}
						}
					}

					if(this->LoadEngineAddressActions(pAddrCur, pAddrSection, (char *)sAddressActionsError, sizeof(sAddressActionsError)))
					{
						this->SetAddress(pszAddressName, pAddrCur);
					}
					else if(psError)
					{
						snprintf(psError, nMaxLength, "Failed to get \"%s\" address action: %s\n", pszAddressName, sAddressActionsError);
					}
				}
				else if(psError)
				{
					snprintf(psError, nMaxLength, "Failed to get \"%s\" signature in \"%s\" address", pszSignatureName, pszAddressName);
				}
			}
			else if(psError)
			{
				snprintf(psError, nMaxLength, "Failed to get signature (\"%s\" key) at \"%s\" address", pszSignatureKey, pszAddressName);
			}

			pAddrSection = pAddrSection->GetNextKey();
		}
		while(pAddrSection);
	}
	else if(psError)
	{
		strncpy(psError, "Signatures section is empty", nMaxLength);
	}

	return bResult;
}

bool EntityManager::GameData::Config::LoadEngineAddressActions(uintptr_t &pAddrCur, KeyValues *pActionSection, char *psError, size_t nMaxLength)
{
	KeyValues *pAction = pActionSection->GetFirstSubKey();

	bool bResult = pAction != nullptr;

	if(bResult)
	{
		do
		{
			const char *pszName = pAction->GetName();

			ptrdiff_t nActionValue = EntityManager::GameData::ReadOffset(pAction->GetString(NULL));

			if(!strcmp(pszName, "offset"))
			{
				pAddrCur += nActionValue;
			}
			else if(!strncmp(pszName, "read", 4))
			{
				if(!pszName[4])
				{
					pAddrCur = *(uintptr_t *)(pAddrCur + nActionValue);
				}
				else if(!strcmp(&pszName[4], "_offs32"))
				{
					pAddrCur = pAddrCur + nActionValue + sizeof(int32_t) + *(int32_t *)(pAddrCur + nActionValue);
				}
				else if(psError)
				{
					bResult = false;
					snprintf(psError, nMaxLength, "Unknown \"%s\" read action", pszName);
				}
			}
			else if(!strcmp(pszName, EntityManager::GameData::GetCurrentPlatformName()))
			{
				bResult = this->LoadEngineAddressActions(pAddrCur, pAction, psError, nMaxLength); // Recursive by platform.
			}
			else if(psError)
			{
				bResult = false;
				snprintf(psError, nMaxLength, "Unknown \"%s\" action", pszName);
			}

			pAction = pAction->GetNextKey();
		}
		while(pAction);
	}

	return bResult;
}

CMemory EntityManager::GameData::Config::GetAddress(const std::string &sName) const
{
	auto itResult = this->m_aAddressMap.find(sName);

	if(itResult != this->m_aAddressMap.cend())
	{
		CMemory pResult = itResult->second;

		g_pEntityManagerLogger->DevMessageFormat(1, "Address (or signature) \"%s\" is %p", sName.c_str(), (void *)pResult);

		return pResult;
	}

	g_pEntityManagerLogger->DevWarningFormat(1, "Address (or signature) \"%s\" is not found", sName.c_str());

	return nullptr;
}


ptrdiff_t EntityManager::GameData::Config::GetOffset(const std::string &sName) const
{
	auto itResult = this->m_aOffsetMap.find(sName);

	if(itResult != this->m_aOffsetMap.cend())
	{
		ptrdiff_t nResult = itResult->second;

		g_pEntityManagerLogger->DevMessageFormat(1, "Offset \"%s\" is 0x%zX (%zd)", sName.c_str(), nResult, nResult);

		return nResult;
	}

	g_pEntityManagerLogger->DevWarningFormat(1, "Offset \"%s\" is not found", sName.c_str());

	return -1;
}

void EntityManager::GameData::Config::SetAddress(const std::string &sName, const CMemory &aMemory)
{
	this->m_aAddressMap[sName] = aMemory;
}

void EntityManager::GameData::Config::SetOffset(const std::string &sName, const ptrdiff_t nValue)
{
	this->m_aOffsetMap[sName] = nValue;
}

void EntityManager::GameData::Config::Clear()
{
	this->m_aAddressMap.clear();
	this->m_aOffsetMap.clear();
}
