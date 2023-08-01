# Define a function to pull the GitHub project
set(SPLAT_GIT_URL "https://github.com/neeilan/splat.git")

set(CLONE_DIR ${CMAKE_CURRENT_BINARY_DIR}/splat)
if(NOT EXISTS ${CLONE_DIR})
    message(STATUS "Cloning project from ${SPLAT_GIT_URL}")
    execute_process(
        COMMAND git clone ${SPLAT_GIT_URL}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RESULT_VARIABLE GIT_RESULT
    )

    if (NOT GIT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone splat from ${SPLAT_GIT_URL}")
    endif ()

    message(STATUS "Successfully pulled splat from ${SPLAT_GIT_URL}")
else()
    message(STATUS "splat directory already exists - not cloning")
endif()

message(STATUS "Running build.sh in ${CLONE_DIR}")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E chdir ${CLONE_DIR} ./build.sh
    RESULT_VARIABLE BUILD_RESULT
)

execute_process(
    COMMAND mv splat/splat bin/splat
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

if(NOT BUILD_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to run build.sh in ${CLONE_DIR}")
endif()

message(STATUS "Successfully built splat in ${CLONE_DIR}")