#include "gamedata.h"

#include <stdlib.h>

#include <tier1/utlstring.h>

#include <entity2/entitysystem.h>
#include <tier0/dbg.h>
#include <tier0/platform.h>
#include <tier1/KeyValues.h>

#define GAMEDATA_FOLDER_DIR "gamedata"
#define GAMEDATA_ENTITY_MANAGER_FILENAME "entity_system.games.txt"

extern IVEngineServer *engine;
extern IFileSystem *filesystem;
extern IServerGameDLL *server;
extern CGameEntitySystem *g_pEntitySystem;

CModule g_aLibEngine, 
        g_aLibServer;

bool EntityManagerSpace::GameData::Init(char *psError, size_t nMaxLength)
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

bool EntityManagerSpace::GameData::Load(const char *pszBasePath, char *psError, size_t nMaxLength)
{
	bool bResult = true;

	KeyValues *pGamesValues = new KeyValues("Games");

	{
		char sConfigFile[MAX_PATH];

		snprintf(sConfigFile, sizeof(sConfigFile), 
#ifdef PLATFORM_WINDOWS
		                                           "%s\\%s\\%s", 
#else
		                                           "%s/%s/%s", 
#endif
		                                           pszBasePath, GAMEDATA_FOLDER_DIR, GAMEDATA_ENTITY_MANAGER_FILENAME);

		bResult = pGamesValues->LoadFromFile(filesystem, (const char *)sConfigFile);

		if(!bResult && psError)
		{
			snprintf(psError, nMaxLength, "Can't to load KeyValue from \"%s\" file", sConfigFile);
		}
	}

	if(bResult)
	{
		char sEntityManagerlError[1024];

		bResult = this->InternalEntitySystemLoad(pGamesValues, sEntityManagerlError, sizeof(sEntityManagerlError));

		if(!bResult && psError)
		{
			snprintf(psError, nMaxLength, "Failed to load a entity manager: %s", sEntityManagerlError);
		}
	}

	delete pGamesValues;

	return bResult;
}

void EntityManagerSpace::GameData::Clear()
{
	this->m_aEntitySystemConfig.Clear();
}

void EntityManagerSpace::GameData::Destroy()
{
	// ...
}


CMemory EntityManagerSpace::GameData::GetEntitySystemAddress(const std::string &sName)
{
	CMemory pResult = this->m_aEntitySystemConfig.GetAddress(sName);

	DebugMsg("Address (or signature) \"%s\" is %p\n", sName.c_str(), (void *)pResult);

	return pResult;
}

ptrdiff_t EntityManagerSpace::GameData::GetEntitySystemOffset(const std::string &sName)
{
	ptrdiff_t nResult = this->m_aEntitySystemConfig.GetOffset(sName);

	DebugMsg("Offset \"%s\" is 0x%tX (%td)\n", sName.c_str(), nResult, nResult);

	return nResult;
}

const char *EntityManagerSpace::GameData::GetSourceEngineName()
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

const char *EntityManagerSpace::GameData::GetPlatformName()
{
#if defined(_WINDOWS)
#	if defined(X64BITS)
	return "windows64";
#	else
	return "windows";
#	endif
#elif defined(_LINUX)
#	if defined(X64BITS)
	return "linux64";
#	else
	return "linux";
#	endif
#else
#	error Unsupported platform
	return "unknown";
#endif
}

ptrdiff_t EntityManagerSpace::GameData::ReadOffset(const char *pszValue)
{
	return static_cast<ptrdiff_t>(strtol(pszValue, NULL, 0));
}

bool EntityManagerSpace::GameData::InternalEntitySystemLoad(KeyValues *pGamesValues, char *psError, size_t nMaxLength)
{
	const char *pszEngineName = this->GetSourceEngineName();

	KeyValues *pEngineValues = pGamesValues->FindKey(pszEngineName, false);

	bool bResult = pEngineValues != nullptr;

	if(bResult)
	{
		this->InternalLoad(pEngineValues, this->m_aEntitySystemConfig, psError, nMaxLength);
	}
	else if(!psError)
	{
		snprintf(psError, nMaxLength, "Failed to find \"%s\" section", pszEngineName);
	}

	return bResult;
}

bool EntityManagerSpace::GameData::InternalLoad(KeyValues *pEngineValues, Config &aStorage, char *psError, size_t nMaxLength)
{
	KeyValues *pSectionValues = pEngineValues->FindKey("Signatures", false);

	bool bResult = true;

	if(pSectionValues) // Ignore the section not found for result.
	{
		bResult = this->InternalLoadSignatures(pSectionValues, aStorage, psError, nMaxLength);

		if(bResult)
		{
			pSectionValues = pEngineValues->FindKey("Offsets", false);

			if(pSectionValues) // Same ignore.
			{
				bResult = this->InternalLoadOffsets(pSectionValues, aStorage, psError, nMaxLength);
			}
		}
	}

	return bResult;
}

bool EntityManagerSpace::GameData::InternalLoadSignatures(KeyValues *pSignaturesValues, Config &aStorage, char *psError, size_t nMaxLength)
{
	KeyValues *pSigSection = pSignaturesValues->GetFirstSubKey();

	bool bResult = pSigSection != nullptr;

	if(bResult)
	{
		const char *pszLibraryKey = "library", 
		           *pszPlatformKey = this->GetPlatformName();

		do
		{
			const char *pszSigName = pSigSection->GetName();

			KeyValues *pLibraryValues = pSigSection->FindKey(pszLibraryKey, false);

			bResult = pLibraryValues != nullptr;

			if(bResult)
			{
				const char *pszLibraryName = pLibraryValues->GetString(nullptr, "unknown");

				auto itResult = this->m_aLibraryMap.find(std::string(pszLibraryName));

				bResult = itResult != this->m_aLibraryMap.cend();

				if(bResult)
				{
					const CModule *pLibModule = itResult->second;

					KeyValues *pPlatformValues = pSigSection->FindKey(pszPlatformKey, false);

					bResult = pPlatformValues != nullptr;

					if(pPlatformValues)
					{
						const char *pszSignature = pPlatformValues->GetString(nullptr);

						CMemory aSigResult = pLibModule->FindPatternSIMD(pszSignature);

						bResult = (bool)aSigResult;

						if(bResult)
						{
							aStorage.SetAddress(pszSigName, aSigResult);
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

bool EntityManagerSpace::GameData::InternalLoadOffsets(KeyValues *pOffsetsValues, Config &aStorage, char *psError, size_t nMaxLength)
{
	KeyValues *pOffsetSection = pOffsetsValues->GetFirstSubKey();

	bool bResult = pOffsetSection != nullptr;

	if(bResult)
	{
		const char *pszPlatformKey = this->GetPlatformName();

		do
		{
			const char *pszOffsetName = pOffsetSection->GetName();

			KeyValues *pPlatformValues = pOffsetSection->FindKey(pszPlatformKey, false);

			bResult = pPlatformValues != nullptr;

			if(pPlatformValues)
			{
				aStorage.SetOffset(std::string(pszOffsetName), EntityManagerSpace::GameData::ReadOffset(pPlatformValues->GetString(nullptr)));
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


CMemory EntityManagerSpace::GameData::Config::GetAddress(const std::string &aName) const
{
	auto itResult = this->m_aAddressMap.find(aName);

	if(itResult != this->m_aAddressMap.cend())
	{
		return itResult->second;
	}

	return nullptr;
}


ptrdiff_t EntityManagerSpace::GameData::Config::GetOffset(const std::string &aName) const
{
	auto itResult = this->m_aOffsetMap.find(aName);

	if(itResult != this->m_aOffsetMap.cend())
	{
		return itResult->second;
	}

	return -1;
}

void EntityManagerSpace::GameData::Config::SetAddress(const std::string &aName, const CMemory &aMemory)
{
	this->m_aAddressMap[aName] = aMemory;
}

void EntityManagerSpace::GameData::Config::SetOffset(const std::string &aName, const ptrdiff_t nValue)
{
	this->m_aOffsetMap[aName] = nValue;
}

void EntityManagerSpace::GameData::Config::Clear()
{
	this->m_aAddressMap.clear();
	this->m_aOffsetMap.clear();
}
