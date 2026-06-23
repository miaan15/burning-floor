# Only support Clang and libc++ (for now, till all support c++ modules) // TODO

cmake_minimum_required(VERSION 3.29)

# Check Clang
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/require_clang.cmake)

project(burning-floor LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Check libc++
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/require_libcxx.cmake)

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

add_compile_options(-ftime-trace)

if(CMAKE_BUILD_TYPE STREQUAL "Debug ")
    message(STATUS "Debug build detected: Enabling strict warnings.")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Wcast-align
        -Wnull-dereference
        -Wdouble-promotion
    )
endif()

add_compile_options(-fno-rtti -fno-exceptions) # No exception & run-time type info

# ----------------------------------------------------------------------------------------
# Vendors
# ----------------------------------------------------------------------------------------
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/vendor_sdl.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/vendor_glm.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/vendor_box2d.cmake)

# ----------------------------------------------------------------------------------------
# Vars
# ----------------------------------------------------------------------------------------
set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/vendor)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIRS
    ${SDL_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${BOX2D_INCLUDE_DIR})

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE SRC_CPPM CONFIGURE_DEPENDS "${SRC_DIR}/*.cppm")

set(LIBS
    ${SDL_TARGET}
    ${GLM_TARGET}
    ${BOX2D_TARGET})

# ----------------------------------------------------------------------------------------
# Final
# ----------------------------------------------------------------------------------------
set(ALL_MODULE_SRCS
    ${SRCS}
    ${SRC_CPPM}
    ${GLM_MODULE_SOURCE})
list(REMOVE_ITEM ALL_MODULE_SRCS "${SRC_DIR}/main.cpp")

add_executable(game "${SRC_DIR}/main.cpp")
target_sources(game PRIVATE FILE_SET CXX_MODULES FILES ${ALL_MODULE_SRCS})

target_include_directories(game PRIVATE ${INC_DIRS})
target_link_libraries(game PRIVATE ${LIBS})
