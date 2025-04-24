#ifndef _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PATH_RESOLVER_HPP_
#	define _INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PATH_RESOLVER_HPP_

#	define ENTITY_MANAGER_ADDONS_DIR "addons"
#	define ENTITY_MANAGER_BINARY_DIR "bin"

#	include <stddef.h>

#	include <string>
#	include <string_view>

#	include <dynlibutils/module.hpp>

namespace EntityManager
{
	class PathResolver
	{
	public:
		PathResolver(void *pInitModule) : m_pModule(pInitModule) {}

	public:
		bool Init(char *psError = NULL, size_t nMaxLength = 0) { return m_aModule.InitFromMemory(m_pModule); }

	public:
		std::string_view GetAbsoluteModuleFilename() { return m_aModule.GetPath(); }
		std::string_view ExtractSubpath(std::string_view svStartMarker = ENTITY_MANAGER_ADDONS_DIR, std::string_view svEndMarker = ENTITY_MANAGER_BINARY_DIR)
		{
			auto svFullPath = GetAbsoluteModuleFilename();

			std::size_t nStartPosition = svFullPath.find(svStartMarker);

			if(nStartPosition != std::string_view::npos)
			{
				std::size_t nEndPosition = svFullPath.find(svEndMarker, nStartPosition);

				if(nEndPosition != std::string_view::npos)
				{
					return svFullPath.substr(nStartPosition, nEndPosition - (nStartPosition + 1));
				}
			}

			return std::string_view();
		}

	private:
		void *m_pModule;
		DynLibUtils::CModule m_aModule;

		std::string_view m_sModuleFilename;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PATH_RESOLVER_HPP_
