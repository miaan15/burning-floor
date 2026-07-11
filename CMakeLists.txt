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
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/vendor_sdl.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/vendor_box2d.cmake)

# ----------------------------------------------------------------------------------------
# Vars
# ----------------------------------------------------------------------------------------
set(VENDOR_DIR ${CMAKE_SOURCE_DIR}/vendor)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INC_DIRS
    ${SRC_DIR}
    ${SDL_INCLUDE_DIR}
    ${BOX2D_INCLUDE_DIR})

file(GLOB_RECURSE C_SRCS CONFIGURE_DEPENDS "${SRC_DIR}/*.c")

set(LIBS
    ${SDL_TARGET}
    ${BOX2D_TARGET})

# ----------------------------------------------------------------------------------------
# Final
# ----------------------------------------------------------------------------------------
add_executable(a ${C_SRCS})
target_include_directories(a PRIVATE ${INC_DIRS})
target_link_libraries(a PRIVATE ${LIBS})

target_compile_definitions(a PRIVATE
    PROJECT_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
