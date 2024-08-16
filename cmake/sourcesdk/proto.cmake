# Entity Manger
# Copyright (C) 2023-2024 Wend4r
# Licensed under the GPLv3 license. See LICENSE file in the project root for details.
## Generate protobuf source & headers

if(LINUX)
	set(SOURCESDK_PROTOC_EXECUTABLE ${SOURCESDK_DIR}/devtools/bin/linux/protoc)
	set(SOURCESDK_PROTOBUF_STATIC_LIBRARY ${SOURCESDK_LIB_DIR}/linux64/release/libprotobuf.a)
elseif(WINDOWS)
	set(SOURCESDK_PROTOC_EXECUTABLE ${SOURCESDK_DIR}/devtools/bin/protoc.exe)
	set(SOURCESDK_PROTOBUF_STATIC_LIBRARY ${SOURCESDK_LIB_DIR}/public/win64/2015/libprotobuf.lib)
endif()

set(SOURCESDK_PROTOBUF_DIR "${SOURCESDK_DIR}/thirdparty/protobuf-3.21.8")
set(SOURCESDK_PROTOBUF_SOURCE_DIR "${SOURCESDK_PROTOBUF_DIR}/src")
list(APPEND SOURCESDK_INCLUDE_DIR ${SOURCESDK_PROTOBUF_SOURCE_DIR})

set(SOURCESDK_PROTO_FILENAME_FILES
	${SOURCESDK_PROTO_FILENAME_FILES}
)

foreach(PROTO_FILE IN LISTS SOURCESDK_PROTO_FILES)
	list(APPEND SOURCESDK_PROTO_FILENAME_FILES "${PROTO_FILE}.proto")
endforeach()

execute_process(
	COMMAND ${CMAKE_COMMAND} -E echo "Generating Protobuf files"
	COMMAND ${SOURCESDK_PROTOC_EXECUTABLE} --proto_path=${SOURCESDK_PROTOBUF_SOURCE_DIR} --proto_path=common --cpp_out=common ${SOURCESDK_PROTO_FILENAME_FILES}
	WORKING_DIRECTORY ${SOURCESDK_DIR}
)

set(SOURCESDK_PROTO_SOURCE_FILES
	${SOURCESDK_PROTO_SOURCE_FILES}
)

foreach(PROTO_FILE IN LISTS SOURCESDK_PROTO_FILES)
	list(APPEND SOURCESDK_PROTO_SOURCE_FILES "${PROTO_FILE}.pb.cc")
endforeach()
