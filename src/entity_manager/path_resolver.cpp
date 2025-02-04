#include <entity_manager/path_resolver.hpp>

#include <stddef.h>
#include <string.h>

#include <cstddef>

#include <dynlibutils/module.hpp>

#include <tier0/basetypes.h>
#include <tier0/dbg.h>

EntityManager::PathResolver::PathResolver(const void *pInitModule)
 :  m_pModule(pInitModule)
{
}

bool EntityManager::PathResolver::Init(char *psError, size_t nMaxLength)
{
	m_aModule.InitFromMemory(m_pModule);

	return true;
}

std::string_view EntityManager::PathResolver::GetAbsoluteModuleFilename()
{
	return m_aModule.GetModulePath();
}

std::string_view EntityManager::PathResolver::ExtractSubpath(std::string_view sStartMarker, std::string_view sEndMarker)
{
	auto sFullPath = GetAbsoluteModuleFilename();

	std::size_t nStartPosition = sFullPath.find(sStartMarker);

	if(nStartPosition != std::string_view::npos)
	{
		std::size_t nEndPosition = sFullPath.find(sEndMarker, nStartPosition);

		if(nEndPosition != std::string_view::npos)
		{
			return sFullPath.substr(nStartPosition, nEndPosition - (nStartPosition + 1));
		}
	}

	return "";
}
