#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_GAMEDATA_H_
#define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_GAMEDATA_H_

#include <stddef.h>
#include <map>
#include <string>

#include "memory_utils/module.h"
#include "memory_utils/memaddr.h"

class KeyValues;

namespace EntityManagerSpace
{
	class GameData
	{
		typedef GameData ThisClass;

	public:
		bool Init(char *psError, size_t nMaxLength);
		bool Load(const char *pszBasePath, char *psError = NULL, size_t nMaxLength = 0);
		void Clear();
		void Destroy();

	public:
		const CModule *FindLibrary(const char *pszName);

	protected:
		bool LoadEntityKeyValues(const char *pszBaseGameConfigDir, char *psError = NULL, size_t nMaxLength = 0);
		bool LoadEntitySystem(const char *pszBaseGameConfigDir, char *psError = NULL, size_t nMaxLength = 0);
		bool LoadSpawnGroup(const char *pszBaseGameConfigDir, char *psError = NULL, size_t nMaxLength = 0);

	public:
		CMemory GetEntityKeyValuesAddress(const std::string &sName);
		ptrdiff_t GetEntityKeyValuesOffset(const std::string &sName);

		CMemory GetEntitySystemAddress(const std::string &sName);
		ptrdiff_t GetEntitySystemOffset(const std::string &sName);

		CMemory GetSpawnGroupAddress(const std::string &sName);
		ptrdiff_t GetSpawnGroupOffset(const std::string &sName);

	protected:
		static const char *GetSourceEngineName();

		enum Platform : int
		{
			UNKNOWN = -1,
			PLATFORM_FIRST = 0,

			WINDOWS = 0,
			WINDOWS64,

			LINUX,
			LINUX64,

			MAC,
			MAC64,

			PLATFORM_MAX
		};

		inline static Platform GetCurrentPlatform();
		inline static const char *GetCurrentPlatformName();
		inline static const char *GetPlatformName(Platform eElm);

		static ptrdiff_t ReadOffset(const char *pszValue);

	private:
		class Config
		{
		public:
			bool Load(KeyValues *pGamesValues, char *psError = NULL, size_t nMaxLength = 0);
			bool LoadEngine(KeyValues *pEngineValues, char *psError = NULL, size_t nMaxLength = 0);

			bool LoadEngineSignatures(KeyValues *pSignaturesValues, char *psError = NULL, size_t nMaxLength = 0);
			bool LoadEngineOffsets(KeyValues *pOffsetsValues, char *psError = NULL, size_t nMaxLength = 0);

			// Step #2 - addresses.
			bool LoadEngineAddresses(KeyValues *pAddressesValues, char *psError = NULL, size_t nMaxLength = 0);
			bool LoadEngineAddressActions(uintptr_t &pAddrCur, KeyValues *pActionValues, char *psError = NULL, size_t nMaxLength = 0);

			void Clear();

		public:
			CMemory GetAddress(const std::string &sName) const;
			ptrdiff_t GetOffset(const std::string &sName) const;

		protected:
			void SetAddress(const std::string &sName, const CMemory &aMemory);
			void SetOffset(const std::string &sName, const ptrdiff_t nValue);

		private:
			std::map<std::string, CMemory> m_aAddressMap;
			std::map<std::string, ptrdiff_t> m_aOffsetMap;
		};

	private:
		std::map<std::string, const CModule *> m_aLibraryMap;

		Config m_aEntityKeyValuesConfig;
		Config m_aEntitySystemConfig;
		Config m_aSpawnGroupConfig;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_GAMEDATA_H_
