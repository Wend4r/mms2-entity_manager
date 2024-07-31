if(NOT ANY_CONFIG_DIR)
	message(FATAL_ERROR "ANY_CONFIG_DIR is empty")
endif()

set(ANY_CONFIG_BINARY_DIR "any_config")
set(ANY_CONFIG_INCLUDE_DIR "${ANY_CONFIG_DIR}/include")

add_subdirectory(${ANY_CONFIG_DIR} ${ANY_CONFIG_BINARY_DIR})
