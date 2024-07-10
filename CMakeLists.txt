cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

if(POLICY CMP0092)
	cmake_policy(SET CMP0092 NEW) # Don't add -W3 warning level by default.
endif()


project(entity_manager
	VERSION 1.0.1
	DESCRIPTION "Entity Manager"
	HOMEPAGE_URL "https://github.com/Wend4r/mms-entity_manager"
	LANGUAGES C CXX ASM
)

string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWER)
string(TOUPPER "${PROJECT_NAME}" PROJECT_NAME_UPPER)

set(PROJECT_OUTPUT_NAME "entitymgr")

set(PROJECT_AUTHOR "Wend4r")
set(PROJECT_DESCRIPTION_FULL "MM:Source plugin to manage entities")
set(PROJECT_LICENSE "GPLv3")
string(TIMESTAMP PROJECT_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP PROJECT_BUILD_TIME "%H:%M:%S")

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
set(SOURCE_PROJECT_DIR "${SOURCE_DIR}/${PROJECT_NAME}")
set(EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external")
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
include("cmake/dynlibutils.cmake")
include("cmake/gamedata.cmake")
include("cmake/logger.cmake")
include("cmake/metamod.cmake")
include("cmake/sourcesdk.cmake")

set(SOURCE_FILES
	${SOURCE_DIR}/${PROJECT_NAME}/provider_agent/resourcemanifest.cpp
	${SOURCE_DIR}/${PROJECT_NAME}/provider_agent/spawngroup.cpp
	${SOURCE_DIR}/${PROJECT_NAME}/provider/gamedata/entitysystem.cpp
	${SOURCE_DIR}/${PROJECT_NAME}/provider/gamedata/gameresource.cpp
	${SOURCE_DIR}/${PROJECT_NAME}/provider/gamedata/gamesystem.cpp
	${SOURCE_DIR}/${PROJECT_NAME}/provider/gamedata/source2server.cpp
	${SOURCE_DIR}/${PROJECT_NAME}/provider/gamedata/spawngroup.cpp
	${SOURCE_DIR}/${PROJECT_NAME}/provider/entitysystem.cpp
	${SOURCE_DIR}/${PROJECT_NAME}/provider/gameresource.cpp
	${SOURCE_DIR}/${PROJECT_NAME}/provider/gamesystem.cpp
	${SOURCE_DIR}/${PROJECT_NAME}/provider/source2server.cpp
	${SOURCE_DIR}/${PROJECT_NAME}/provider/spawngroup.cpp
	${SOURCE_DIR}/${PROJECT_NAME}/provider_agent.cpp
	${SOURCE_DIR}/${PROJECT_NAME}/provider.cpp
	${SOURCE_DIR}/${PROJECT_NAME}/settings.cpp
	${SOURCE_DIR}/${PROJECT_NAME}.cpp
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
target_include_directories(${PROJECT_NAME} PRIVATE ${DYNLIBUTILS_INCLUDE_DIR} ${DYNOHOOK_INCLUDE_DIR} ${GAMEDATA_INCLUDE_DIR} ${LOGGER_INCLUDE_DIR} ${METAMOD_INCLUDE_DIR} ${SOURCESDK_INCLUDE_DIR} ${SOURCE_PROJECT_DIR} ${SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${DYNLIBUTILS_BINARY_DIR} ${DYNOHOOK_BINARY_DIR} ${GAMEDATA_BINARY_DIR} ${LOGGER_BINARY_DIR} ${SOURCESDK_LIBRARIES})
