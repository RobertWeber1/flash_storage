cmake_minimum_required(VERSION 3.0)

project("flash_storage" CXX)


################################################################################


execute_process(
	COMMAND git describe --tags
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_STRIP_TRAILING_WHITESPACE
	OUTPUT_VARIABLE GIT_TAG
)

execute_process(
	COMMAND git rev-parse HEAD
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_STRIP_TRAILING_WHITESPACE
	OUTPUT_VARIABLE COMMIT_HASH
)

configure_file (
	"${PROJECT_SOURCE_DIR}/src/version.cpp.in"
	"${PROJECT_BINARY_DIR}/src/version.cpp"
)

set( VERSION_SRC "${PROJECT_BINARY_DIR}/src/version.cpp" )


################################################################################


add_library( ${PROJECT_NAME}
	${VERSION_SRC}
)

target_include_directories( ${PROJECT_NAME}
	PUBLIC src/
)


################################################################################


add_executable( test-${PROJECT_NAME} EXCLUDE_FROM_ALL
	test/catch.cc
	test/version.cc
	test/layout.cc
	test/encoding.cc
	test/sector_list.cc
	test/functions.cc
	test/flash_storage.cc
)

target_link_libraries( test-${PROJECT_NAME}
	asan
	ubsan
	${PROJECT_NAME}
)

target_compile_options( test-${PROJECT_NAME}
	PRIVATE -Wall
	PRIVATE -Wpedantic
	PRIVATE -Wextra
	PRIVATE -fsanitize=address
	PRIVATE -fsanitize=undefined
	PRIVATE -std=c++14
)
