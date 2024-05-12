if(NOT GAMEDATA_DIR)
	message(FATAL_ERROR "GAMEDATA_DIR is empty")
endif()

set(GAMEDATA_BINARY_DIR "gamedata")
set(GAMEDATA_INCLUDE_DIR "${GAMEDATA_DIR}/include")

add_subdirectory(${GAMEDATA_DIR} ${GAMEDATA_BINARY_DIR})
