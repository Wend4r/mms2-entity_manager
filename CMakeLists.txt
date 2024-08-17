# Entity Manager
# Copyright (C) 2023-2024 Wend4r
# Licensed under the GPLv3 license. See LICENSE file in the project root for details.

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
	cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()


project(mms2-entity_manager
	VERSION 1.0.1
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

if(SOURCESDK_DIR)
	file(TO_CMAKE_PATH "${SOURCESDK_DIR}" SOURCESDK_DIR)
else()
	set(SOURCESDK_DIR "${EXTERNAL_DIR}/hl2sdk")
endif()

if(METAMOD_DIR)
	file(TO_CMAKE_PATH "${METAMOD_DIR}" METAMOD_DIR)
else()
	set(METAMOD_DIR "${EXTERNAL_DIR}/metamod-source")
endif()

if(ANY_CONFIG_DIR)
	file(TO_CMAKE_PATH "${ANY_CONFIG_DIR}" ANY_CONFIG_DIR)
else()
	set(ANY_CONFIG_DIR "${EXTERNAL_DIR}/any_config")
endif()

if(DYNLIBUTILS_DIR)
	file(TO_CMAKE_PATH "${DYNLIBUTILS_DIR}" DYNLIBUTILS_DIR)
else()
	set(DYNLIBUTILS_DIR "${EXTERNAL_DIR}/memory_utils")
endif()

if(DYNOHOOK_DIR)
	file(TO_CMAKE_PATH "${DYNOHOOK_DIR}" DYNOHOOK_DIR)
else()
	set(DYNOHOOK_DIR "${EXTERNAL_DIR}/dynohook")
endif()

if(GAMEDATA_DIR)
	file(TO_CMAKE_PATH "${GAMEDATA_DIR}" GAMEDATA_DIR)
else()
	set(GAMEDATA_DIR "${EXTERNAL_DIR}/gamedata")
endif()

if(LOGGER_DIR)
	file(TO_CMAKE_PATH "${LOGGER_DIR}" LOGGER_DIR)
else()
	set(LOGGER_DIR "${EXTERNAL_DIR}/logger")
endif()

set(SOURCESDK_PROTO_FILES
	common/networkbasetypes
	common/netmessages
	common/network_connection
)

include("cmake/platform/shared.cmake")

if(LINUX)
	include("cmake/platform/linux.cmake")
elseif(WINDOWS)
	include("cmake/platform/windows.cmake")
endif()

# Libraries.
include("cmake/any_config.cmake")
include("cmake/dynlibutils.cmake")
include("cmake/gamedata.cmake")
include("cmake/logger.cmake")
include("cmake/metamod.cmake")
include("cmake/sourcesdk.cmake")
include("cmake/sourcesdk/proto.cmake")

set(SOURCE_FILES
	${SOURCE_PROJECT_DIR}/provider_agent/resourcemanifest.cpp
	${SOURCE_PROJECT_DIR}/provider_agent/spawngroup.cpp
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

add_library(${PROJECT_NAME} SHARED ${SOURCESDK_SOURCE_FILES} ${SOURCE_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_EXTENSIONS OFF)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_OUTPUT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE ${PLATFORM_COMPILER_OPTIONS})
target_link_options(${PROJECT_NAME} PRIVATE ${PLATFORM_LINKER_OPTIONS})

target_compile_definitions(${PROJECT_NAME} PRIVATE ${PLATFORM_COMPILE_DEFINITIONS} ${METAMOD_COMPILE_DEFINITIONS} ${SOURCESDK_COMPILE_DEFINTIONS})
target_include_directories(${PROJECT_NAME} PRIVATE ${ANY_CONFIG_INCLUDE_DIR} ${DYNLIBUTILS_INCLUDE_DIR} ${DYNOHOOK_INCLUDE_DIR} ${GAMEDATA_INCLUDE_DIR} ${LOGGER_INCLUDE_DIR} ${METAMOD_INCLUDE_DIR} ${SOURCESDK_INCLUDE_DIR} ${SOURCE_PROJECT_DIR} ${SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${ANY_CONFIG_BINARY_DIR} ${DYNLIBUTILS_BINARY_DIR} ${DYNOHOOK_BINARY_DIR} ${GAMEDATA_BINARY_DIR} ${LOGGER_BINARY_DIR} ${SOURCESDK_LINK_LIBRARIES})
