message(STATUS "Checking cglm...")

set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
set(CGLM_VENDOR_SOURCE_DIR "${VENDOR_DIR}/cglm")
set(CGLM_VENDOR_BUILD_DIR "${CGLM_VENDOR_SOURCE_DIR}/build")

if(NOT IS_DIRECTORY "${CGLM_VENDOR_SOURCE_DIR}")
    message(STATUS "Cloning cglm to ${CGLM_VENDOR_SOURCE_DIR}")
    execute_process(
        COMMAND bash -c [[
            mkdir -p vendor
        ]]
    )
    execute_process(
        COMMAND bash -c [[
            set -e
            git clone --depth 1 --branch v0.9.6 https://github.com/recp/cglm.git cglm
        ]]
        WORKING_DIRECTORY "${VENDOR_DIR}"
        RESULT_VARIABLE SCRIPT_RESULT
    )
    if(NOT SCRIPT_RESULT EQUAL 0)
        message(FATAL_ERROR "Fetching cglm failed: ${SCRIPT_RESULT}")
        file(REMOVE_RECURSE "${CGLM_VENDOR_SOURCE_DIR}")
    endif()
endif()

add_library(CGLM_VENDOR_LIB SHARED IMPORTED)

set(CGLM_INCLUDE_DIR "${CGLM_VENDOR_SOURCE_DIR}/include")
target_include_directories(CGLM_VENDOR_LIB INTERFACE "${CGLM_INCLUDE_DIR}")

set(CGLM_TARGET "")
set(CGLM_INCLUDE_DIR ${CGLM_INCLUDE_DIR})

message(STATUS "cglm Ready.")
