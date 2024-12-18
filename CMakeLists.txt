# Entity Manager
# Copyright (C) 2023-2024 Wend4r
# Licensed under the GPLv3 license. See LICENSE file in the project root for details.

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
	cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()


project(mms2-entity_manager
	VERSION 1.0.2
	DESCRIPTION "Entity Manager"
	HOMEPAGE_URL "https://github.com/Wend4r/mms2-entity_manager"
	LANGUAGES C CXX ASM
)

string(REGEX REPLACE "^[^-]*-" "" PROJECT_NAME_SUBSTRING "${PROJECT_NAME}")
string(TOLOWER "${PROJECT_NAME_SUBSTRING}" PROJECT_NAME_LOWER)
string(TOUPPER "${PROJECT_NAME_SUBSTRING}" PROJECT_NAME_UPPER)

set(PROJECT_OUTPUT_NAME "entitymgr")

set(PROJECT_AUTHOR "Wend4r")
set(PROJECT_DESCRIPTION_FULL "MM:Source plugin to manage entities")
set(PROJECT_LICENSE "GPLv3")
string(TIMESTAMP PROJECT_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP PROJECT_BUILD_TIME "%H:%M:%S")

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")	
set(SOURCE_PROJECT_DIR "${SOURCE_DIR}/${PROJECT_NAME_SUBSTRING}")
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/public")

set(INCLUDE_DIRS
	${INCLUDE_DIR}
	${PUBLIC_DIR}
)

function(set_or_external_dir VAR_NAME DEFAULT_DIR)
	if(${VAR_NAME})
		file(TO_CMAKE_PATH "${${VAR_NAME}}" ${VAR_NAME})
	else()
		set(${VAR_NAME} "${EXTERNAL_DIR}/${DEFAULT_DIR}")
	endif()

	set(${VAR_NAME} "${${VAR_NAME}}" PARENT_SCOPE)
endfunction()

set_or_external_dir(SOURCESDK_DIR "hl2sdk")
set_or_external_dir(METAMOD_DIR "metamod-source")
set_or_external_dir(ANY_CONFIG_DIR "any_config")
set_or_external_dir(DYNLIBUTILS_DIR "memory_utils")
set_or_external_dir(DYNOHOOK_DIR "dynohook")
set_or_external_dir(GAMEDATA_DIR "gamedata")
set_or_external_dir(LOGGER_DIR "logger")

set(SOURCESDK_PROTOS
	networkbasetypes
	netmessages
	network_connection
)

include(cmake/platform/shared.cmake)

# Libraries.
include(cmake/any_config.cmake)
include(cmake/dynlibutils.cmake)
include(cmake/gamedata.cmake)
include(cmake/logger.cmake)
include(cmake/metamod.cmake)
include(cmake/sourcesdk.cmake)

set(SOURCE_FILES
	${SOURCE_PROJECT_DIR}/provider_agent/resourcemanifest.cpp
	${SOURCE_PROJECT_DIR}/provider_agent/spawngroup.cpp
	${SOURCE_PROJECT_DIR}/provider/gamedata/entityresourcemanifest.cpp
	${SOURCE_PROJECT_DIR}/provider/gamedata/entitysystem.cpp
	${SOURCE_PROJECT_DIR}/provider/gamedata/gameresource.cpp
	${SOURCE_PROJECT_DIR}/provider/gamedata/gamesystem.cpp
	${SOURCE_PROJECT_DIR}/provider/gamedata/source2server.cpp
	${SOURCE_PROJECT_DIR}/provider/gamedata/spawngroup.cpp
	${SOURCE_PROJECT_DIR}/provider/entitysystem.cpp
	${SOURCE_PROJECT_DIR}/provider/gameresource.cpp
	${SOURCE_PROJECT_DIR}/provider/gamesystem.cpp
	${SOURCE_PROJECT_DIR}/provider/source2server.cpp
	${SOURCE_PROJECT_DIR}/provider/spawngroup.cpp
	${SOURCE_PROJECT_DIR}/provider_agent.cpp
	${SOURCE_PROJECT_DIR}/provider.cpp
	${SOURCE_PROJECT_DIR}/settings.cpp
	${SOURCE_PROJECT_DIR}.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${SOURCESDK_SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD 17
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF

	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_OUTPUT_NAME})

if(WINDOWS)
	set_target_properties(${PROJECT_NAME} PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(MACOS)
	set_target_properties(${PROJECT_NAME} PROPERTIES OSX_ARCHITECTURES "x86_64")
endif()

target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILER_OPTIONS} ${PLATFORM_COMPILER_OPTIONS} ${COMPILE_OPTIONS} ${SOURCESDK_COMPILE_OPTIONS})
target_link_options(${PROJECT_NAME} PRIVATE ${LINKER_OPTIONS} ${PLATFORM_LINKER_OPTIONS} ${SOURCESDK_LINK_OPTIONS})

target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMPILER_DEFINITIONS} ${PLATFORM_COMPILE_DEFINITIONS} ${METAMOD_COMPILE_DEFINITIONS} ${SOURCESDK_COMPILE_DEFINTIONS})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS} ${ANY_CONFIG_INCLUDE_DIR} ${DYNLIBUTILS_INCLUDE_DIR} ${DYNOHOOK_INCLUDE_DIR} ${GAMEDATA_INCLUDE_DIR} ${LOGGER_INCLUDE_DIR} ${METAMOD_INCLUDE_DIR} ${SOURCESDK_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LINK_LIBRARIES} ${ANY_CONFIG_BINARY_DIR} ${DYNLIBUTILS_BINARY_DIR} ${DYNOHOOK_BINARY_DIR} ${GAMEDATA_BINARY_DIR} ${LOGGER_BINARY_DIR} ${SOURCESDK_BINARY_DIR})
