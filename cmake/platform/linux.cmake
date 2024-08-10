# Warnings
set(PLATFORM_COMPILER_OPTIONS
	-Wall -Wno-uninitialized -Wno-switch -Wno-unused
	-Wno-conversion-null -Wno-write-strings
	-Wno-invalid-offsetof -Wno-reorder
	-Wno-register

	# Others
	-mfpmath=sse -msse -fno-strict-aliasing
	-fno-threadsafe-statics -fvisibility=default
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(PLATFORM_COMPILER_OPTIONS
		${PLATFORM_COMPILER_OPTIONS}
		-g3 -ggdb
	)
endif()

set(PLATFORM_LINKER_OPTIONS)

set(PLATFORM_COMPILE_DEFINITIONS
	-D_GLIBCXX_USE_CXX11_ABI=0
)
