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
	public:
		bool Init(char *psError, size_t nMaxLength);
		bool Load(const char *pszBasePath, char *psError = NULL, size_t nMaxLength = 0);
		void Clear();
		void Destroy();

	public:
		CMemory GetEntitySystemAddress(const std::string &sName);
		ptrdiff_t GetEntitySystemOffset(const std::string &sName);

	protected:
		const char *GetSourceEngineName();
		const char *GetPlatformName();
		static ptrdiff_t ReadOffset(const char *pszValue);

	protected:
		bool InternalEntitySystemLoad(KeyValues *pGamesValues, char *psError = NULL, size_t nMaxLength = 0);

	private:
		class Config
		{
		public:
			CMemory GetAddress(const std::string &aName) const;
			ptrdiff_t GetOffset(const std::string &aName) const;

			void SetAddress(const std::string &aName, const CMemory &aMemory);
			void SetOffset(const std::string &aName, const ptrdiff_t nValue);

			void Clear();

		private:
			std::map<std::string, CMemory> m_aAddressMap;
			std::map<std::string, ptrdiff_t> m_aOffsetMap;
		};

		bool InternalLoad(KeyValues *pEngineValues, Config &aStorage, char *psError = NULL, size_t nMaxLength = 0);
		bool InternalLoadSignatures(KeyValues *pSignaturesValues, Config &aStorage, char *psError = NULL, size_t nMaxLength = 0);
		bool InternalLoadOffsets(KeyValues *pOffsetsValues, Config &aStorage, char *psError = NULL, size_t nMaxLength = 0);

	private:
		std::map<std::string, const CModule *> m_aLibraryMap;

		Config m_aEntitySystemConfig;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_GAMEDATA_H_
