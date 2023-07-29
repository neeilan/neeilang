cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME neeilang)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-g -Wall -pedantic -Werror -fvisibility=hidden")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

#----------------------------------------------------------------------------
# Define project sources and includes
#----------------------------------------------------------------------------
set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/src)

include_directories(${COMMON_INCLUDES})
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cc)
list(REMOVE_ITEM SRC_FILES ${PROJECT_SOURCE_DIR}/cli.cc)


if(TARGET_X86)
  ###############################
  # x86-64 Backend impl.
  ###############################
  file(GLOB X86_64_BACKEND_FILES ${PROJECT_SOURCE_DIR}/src/backends/x86-64/*.cc)
  list(APPEND SRC_FILES ${X86_64_BACKEND_FILES})
  set(X86_64_BACKEND_INCLUDES ${PROJECT_SOURCE_DIR}/src/backends/x86-64)
  include_directories(${X86_64_BACKEND_INCLUDE_DIRS})
  add_compile_definitions(TARGET_X86)
else()
  ###############################
  # LLVM Backend impl.
  ###############################
  file(GLOB LLVM_BACKEND_FILES ${PROJECT_SOURCE_DIR}/src/backends/llvm/*.cc)
  list(APPEND SRC_FILES ${LLVM_BACKEND_FILES})
  include_directories(${LLVM_INCLUDE_DIRS})

  ###############################
  # LLVM libraries
  ###############################
  set(CMAKE_MODULE_PATH
          ${CMAKE_MODULE_PATH}
          "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

  find_package(LLVM REQUIRED)
  include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
  link_directories(${LLVM_LIBRARY_DIRS})
  link_libraries(${LLVM_LIBS})
  set(CMAKE_EXE_LINKER_FLAGS ${LLVM_LD_FLAGS_STRING})
endif()

################################
# Normal Libraries & Executables
################################
add_library(neeilang_lib ${SRC_FILES})
add_executable(neeilang ${PROJECT_SOURCE_DIR}/src/cli.cc)
target_link_libraries(neeilang neeilang_lib)
