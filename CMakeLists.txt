cmake_minimum_required(VERSION 3.29)

project(burning-floor LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ----------------------------------------------------------------------------------------
# Options
# ----------------------------------------------------------------------------------------
# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# Asan
option(USE_ASAN "Enable AddressSanitizer" OFF)

# ----------------------------------------------------------------------------------------
# Platforms
# ----------------------------------------------------------------------------------------
if(WIN32)
    message(STATUS "Platform: Windows")
elseif(APPLE)
    message(STATUS "Platform: macOS / iOS")
elseif(UNIX AND NOT APPLE)
    message(STATUS "Platform: Linux")
else()
    message(WARNING "Platform: Unknown")
endif()

# ----------------------------------------------------------------------------------------
# Flags
# ----------------------------------------------------------------------------------------
if(USE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug ")
    message(STATUS "Debug build detected: Enabling strict warnings.")
    add_compile_options(-g -Wall -Wextra -Wpedantic -Wshadow -Wcast-align -Wnull-dereference)
endif()

add_link_options(-lm)

# ----------------------------------------------------------------------------------------
# Vendors
# ----------------------------------------------------------------------------------------
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/vendors.cmake)

# ----------------------------------------------------------------------------------------
# Vars
# ----------------------------------------------------------------------------------------
set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/vendor)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIRS ${CMAKE_SOURCE_DIR}/src)

set(LIBS SDL)

# ----------------------------------------------------------------------------------------
# Final
# ----------------------------------------------------------------------------------------
set(FINAL a)
add_executable(${FINAL} ${SRC_DIR}/main.c)
target_include_directories(${FINAL} PRIVATE ${INC_DIRS})
target_link_libraries(${FINAL} PRIVATE ${LIBS})

# link with dl lib
if(UNIX)
    target_link_libraries(${FINAL} PRIVATE ${CMAKE_DL_LIBS})
endif()
set_target_properties(${FINAL} PROPERTIES ENABLE_EXPORTS ON)

# function
function(add_module)
    set(options "")
    set(oneArgs TARGET)
    set(mulArgs SOURCE LINK)
    cmake_parse_arguments(ARG "${options}" "${oneArgs}" "${mulArgs}" ${ARGN})

    add_library(${ARG_TARGET} SHARED ${ARG_SOURCE})

    target_include_directories(${ARG_TARGET} PRIVATE ${INC_DIRS})
    target_link_libraries(${ARG_TARGET} PRIVATE ${ARG_LINK})

    if(WIN32)
        set_target_properties(${ARG_TARGET} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
    endif()

    set_target_properties(${ARG_TARGET} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

    if(TARGET a)
        add_dependencies(${FINAL} ${HOT_TARGET})
    endif()
endfunction()

function(add_module_dir)
    set(options "")
    set(oneArgs TARGET)
    set(mulArgs DIRECTORY LINK)
    cmake_parse_arguments(ARG "${options}" "${oneArgs}" "${mulArgs}" ${ARGN})

    if (NOT IS_DIRECTORY ${ARG_DIRECTORY})
        message(FATAL_ERROR "[${ARG_DIRECTORY}] is not directory")
    endif()

    file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS "${ARG_DIRECTORY}/*.c")

    add_module(
        TARGET ${ARG_TARGET}
        SOURCE ${SRCS}
        LINK ${ARG_LINK}
    )
endfunction()

# module
add_module_dir(
    TARGET alloc
    DIRECTORY ${SRC_DIR}/alloc
    LINK ${LIBS}
)

# runtime symbol
target_compile_definitions(a PRIVATE
    _ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

if(WIN32)
    add_custom_command(TARGET a POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${VENDOR_DIR}/sdl/build/SDL3.dll"
            "$<TARGET_FILE_DIR:a>"
    )
endif()
