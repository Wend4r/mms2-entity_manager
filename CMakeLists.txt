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

set(SOURCESDK_PROTO_FILES
	common/networkbasetypes
	common/netmessages
	common/network_connection
)

if(SOURCESDK_DIR)
	file(TO_CMAKE_PATH "${SOURCESDK_DIR}" SOURCESDK_DIR)
else()
	message(WARNING "SOURCESDK_DIR is empty. Will be used default path")
	set(SOURCESDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../hl2sdk-wend4r/hl2sdk-cs2)
endif()

if(METAMOD_DIR)
	file(TO_CMAKE_PATH "${METAMOD_DIR}" METAMOD_DIR)
else()
	message(WARNING "METAMOD_DIR is empty. Will be used default path")
	set(METAMOD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../metamod-source)
endif()

set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SOURCE_PROJECT_DIR "${SOURCE_DIR}/entity_manager")
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")
set(ANY_CONFIG_DIR "${EXTERNAL_DIR}/any_config")
set(DYNLIBUTILS_DIR "${EXTERNAL_DIR}/memory_utils")
set(DYNOHOOK_DIR "${EXTERNAL_DIR}/dynohook")
set(GAMEDATA_DIR "${EXTERNAL_DIR}/gamedata")
set(LOGGER_DIR "${EXTERNAL_DIR}/logger")

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
	${SOURCE_DIR}/entity_manager/provider_agent/resourcemanifest.cpp
	${SOURCE_DIR}/entity_manager/provider_agent/spawngroup.cpp
	${SOURCE_DIR}/entity_manager/provider/gamedata/entitysystem.cpp
	${SOURCE_DIR}/entity_manager/provider/gamedata/gameresource.cpp
	${SOURCE_DIR}/entity_manager/provider/gamedata/gamesystem.cpp
	${SOURCE_DIR}/entity_manager/provider/gamedata/source2server.cpp
	${SOURCE_DIR}/entity_manager/provider/gamedata/spawngroup.cpp
	${SOURCE_DIR}/entity_manager/provider/entitysystem.cpp
	${SOURCE_DIR}/entity_manager/provider/gameresource.cpp
	${SOURCE_DIR}/entity_manager/provider/gamesystem.cpp
	${SOURCE_DIR}/entity_manager/provider/source2server.cpp
	${SOURCE_DIR}/entity_manager/provider/spawngroup.cpp
	${SOURCE_DIR}/entity_manager/provider_agent.cpp
	${SOURCE_DIR}/entity_manager/provider.cpp
	${SOURCE_DIR}/entity_manager/settings.cpp
	${SOURCE_DIR}/entity_manager.cpp
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
