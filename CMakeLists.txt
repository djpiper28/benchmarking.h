cmake_minimum_required(VERSION 3.9)

project(
  benchmarking-framework
  VERSION 0.1
  LANGUAGES C)

set(REPO_URL
    "https://gitlab.cim.rhul.ac.uk/zjac059/benchmarking-homomorphic-encrpytion-libraries"
)

message(STATUS "Building ${CMAKE_PROJECT_NAME}")
message(STATUS "Git repo: ${REPO_URL}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Version: ${CMAKE_PROJECT_VERSION}")
message(STATUS "Build OS: ${CMAKE_SYSTEM_NAME}")

# Get the branch name
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get the latest abbreviated commit hash of the working branch
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Commit info: ${GIT_COMMIT_HASH}@${GIT_BRANCH}")

# Version info
add_definitions("-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
add_definitions("-DGIT_BRANCH=\"${GIT_BRANCH}\"")
add_definitions(
  "-DVERSION=\"${CMAKE_PROJECT_VERSION} (${GIT_COMMIT_HASH}@${GIT_BRANCH})\"")
add_definitions("-DOS=\"${CMAKE_SYSTEM_NAME}\"")
add_definitions("-DREPO_URL=\"${REPO_URL}\"")
add_definitions("-DBUFFER_LENGTH=4096")
add_definitions("-DPROJECT_VERSION=\"${CMAKE_PROJECT_VERSION}\"")
add_definitions("-DPROJECT_NAME=\"${CMAKE_PROJECT_NAME}\"")

# __FILENAME__
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")
add_definitions("-D__FILENAME__=(__FILE__ + SOURCE_PATH_SIZE)")
add_definitions("-DDEBUG")

set(COMPILER_FLAGS
    "-Og -Wno-unused-parameter -Wall -Wextra -Wpedantic -Werror -g")
set(CMAKE_C_FLAGS "${COMPILER_FLAGS}")

# Setup doxygen
find_package(Doxygen REQUIRED dot OPTIONAL_COMPONENTS mscgen dia)

# Compile the library (static build and, unit tests)
set(LIB_SRC
    ./testing.h/ansi_colour.h
    ./testing.h/logger.h
    ./testing.h/logger.c
    ./testing.h/testing.c
    ./testing.h/testing.h
    ./ranges.h
    ./ranges.c
    ./time_utils.h
    ./time_utils.c
    ./mem_profiler.h
    ./mem_profiler.c
    ./bench_output.h
    ./bench_output.c
    ./bench.h
    ./bench.c)
set(TEST_SRC
    ${LIB_SRC}
    ./test_ranges.h
    ./test_ranges.c
    ./test_bench.h
    ./test_bench.c
    ./test_mem_profiler.h
    ./test_mem_profiler.c
    ./test_bench_output.h
    ./test_bench_output.c
    ./tests.c)

set(LINK_LIBS m jansson)

add_library(benchmarking_h ${LIB_SRC})
target_link_libraries(benchmarking_h ${LINK_LIBS})
doxygen_add_docs(benchmarking_h_docs ${LIB_SRC} "Generate documentation pages")
target_include_directories(benchmarking_h PUBLIC 
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/benchmarking_h)

enable_testing()

add_executable(test_benchmarking_h ${TEST_SRC})
target_link_libraries(test_benchmarking_h ${LINK_LIBS})
add_test(test_benchmarking_h test_benchmarking_h)

file(COPY mem_tests.sh DESTINATION ${CMAKE_BINARY_DIR})
file(COPY mem_tests.py DESTINATION ${CMAKE_BINARY_DIR})

# add_test(NAME memory-tests COMMAND bash mem_tests.sh)
