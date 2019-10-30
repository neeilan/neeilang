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

###############################
# LLVM Backend
###############################
file(GLOB LLVM_BACKEND_FILES ${PROJECT_SOURCE_DIR}/src/backend/llvm*.cc)
list(APPEND SRC_FILES ${LLVM_BACKEND_FILES})

set(CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH}
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

set(LLVM_DEP_LIBS LLVMCore)
find_package(LLVM REQUIRED)
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
link_libraries(${LLVM_LIBS})
set(CMAKE_EXE_LINKER_FLAGS ${LLVM_LD_FLAGS_STRING})


set(CMAKE_CXX_STANDARD 11)

################################
# Normal Libraries & Executables
################################
add_library(neeilang_lib ${SRC_FILES})
add_executable(neeilang ${PROJECT_SOURCE_DIR}/src/cli.cc)

target_link_libraries(neeilang neeilang_lib)
