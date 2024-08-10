## Generate protobuf source & headers
if(LINUX)
	set(PROTOC_EXECUTABLE ${SOURCESDK_DIR}/devtools/bin/linux/protoc)
	set(PROTOBUF_STATIC_LIBRARY ${SOURCESDK_LIB_DIR}/linux64/release/libprotobuf.a)
elseif(WINDOWS)
	set(PROTOC_EXECUTABLE ${SOURCESDK_DIR}/devtools/bin/protoc.exe)
	set(PROTOBUF_STATIC_LIBRARY ${SOURCESDK_LIB_DIR}/public/win64/2015/libprotobuf.lib)
endif()

set(PROTO_FILES)

foreach(PROTO_FILE IN LISTS PROTOBUF_FILES)
	list(APPEND PROTO_FILES "${PROTO_FILE}.proto")
endforeach()

execute_process(
	COMMAND ${CMAKE_COMMAND} -E echo "Generating protobuf files"
	COMMAND ${PROTOC_EXECUTABLE} --proto_path=thirdparty/protobuf-3.21.8/src --proto_path=common --cpp_out=common ${PROTO_FILES}
	WORKING_DIRECTORY ${SOURCESDK_DIR}
)

set(PROTO_SOURCE_FILES)

foreach(PROTO_FILE IN LISTS PROTOBUF_FILES)
	set(PROTO_SOURCE_FILES
		${PROTO_SOURCE_FILES}
		"${SOURCESDK_DIR}/${PROTO_FILE}.pb.cc"
	)
endforeach()
