cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME neeilang)
project(${PROJECT_NAME})

set(CMAKE_CXX_FLAGS "-g -Wall -pedantic -Werror")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#----------------------------------------------------------------------------
# Define project sources and includes
#----------------------------------------------------------------------------
set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/src)
include_directories(${COMMON_INCLUDES})
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cc)
list(REMOVE_ITEM SRC_FILES ${PROJECT_SOURCE_DIR}/cli.cc)

set(CMAKE_CXX_STANDARD 11)

################################
# Normal Libraries & Executables
################################
add_library(neeilang_lib ${SRC_FILES})
add_executable(neeilang ${PROJECT_SOURCE_DIR}/src/cli.cc)

target_link_libraries(neeilang neeilang_lib)
