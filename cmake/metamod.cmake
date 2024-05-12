if(NOT METAMOD_DIR)
	message(FATAL_ERROR "METAMOD_DIR is empty")
endif()

set(METAMOD_DEFINITIONS
	-DMETA_PLUGIN_NAME="${PROJECT_NAME}"
)

set(METAMOD_INCLUDE_DIR
	${METAMOD_DIR}/core/sourcehook
	${METAMOD_DIR}/core
)

add_definitions(
	-DMETA_IS_SOURCE2
)
