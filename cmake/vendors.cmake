set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

# SDL
if(NOT IS_DIRECTORY "${VENDOR_DIR}/sdl/build")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -B build
            -DCMAKE_BUILD_TYPE=Release
            -DSDL_TESTS=OFF
        WORKING_DIRECTORY "${VENDOR_DIR}/sdl"
        RESULT_VARIABLE __CONFIG_RES
    )

    execute_process(
        COMMAND ${CMAKE_COMMAND} --build build --config Release
        WORKING_DIRECTORY "${VENDOR_DIR}/sdl"
        RESULT_VARIABLE __BUILD_RES
    )

    if(NOT __BUILD_RES EQUAL 0)
        file(REMOVE_RECURSE "${VENDOR_DIR}/sdl/build")
        message(FATAL_ERROR "Failed to build SDL")
    endif()

    file(WRITE "${VENDOR_DIR}/sdl/build/.gitignore" "*\n")
endif()

add_library(SDL SHARED IMPORTED)
target_include_directories(SDL INTERFACE "${VENDOR_DIR}/sdl/include")

if(WIN32)
    set_target_properties(SDL PROPERTIES
        IMPORTED_LOCATION "${VENDOR_DIR}/sdl/build/SDL3.dll"
        IMPORTED_IMPLIB   "${VENDOR_DIR}/sdl/build/libSDL3.dll.a"
    )
elseif(APPLE)
    set_target_properties(SDL PROPERTIES
        IMPORTED_LOCATION "${VENDOR_DIR}/sdl/build/libSDL3.dylib"
    )
else()
    set_target_properties(SDL PROPERTIES
        IMPORTED_LOCATION "${VENDOR_DIR}/sdl/build/libSDL3.so"
    )
endif()
