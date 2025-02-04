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
		PathResolver(const void *pInitModule);

	public:
		bool Init(char *psError = NULL, size_t nMaxLength = 0);

	public:
		std::string_view GetAbsoluteModuleFilename();
		std::string_view ExtractSubpath(std::string_view sStartMarker = ENTITY_MANAGER_ADDONS_DIR, std::string_view sEndMarker = ENTITY_MANAGER_BINARY_DIR);

	private:
		const void *m_pModule;
		DynLibUtils::CModule m_aModule;

		std::string_view m_sModuleFilename;
	};
};

#endif //_INCLUDE_METAMOD_SOURCE_ENTITY_MANAGER_PATH_RESOLVER_HPP_
