if(NOT LOGGER_DIR)
	message(FATAL_ERROR "LOGGER_DIR is empty")
endif()

set(LOGGER_BINARY_DIR "logger")
set(LOGGER_INCLUDE_DIR "${LOGGER_DIR}/include")

add_subdirectory(${LOGGER_DIR} ${LOGGER_BINARY_DIR})
