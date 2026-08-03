set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

execute_process(
    COMMAND bash -c [[
        mkdir -p vendor
    ]]
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

if(NOT IS_DIRECTORY ${VENDOR_DIR}/sdl)
    execute_process(
        COMMAND bash -c [[
        set -e
            git clone --depth 1 --branch release-3.4.12 https://github.com/miaan15/SDL.git sdl
        ]]
        WORKING_DIRECTORY ${VENDOR_DIR}
        RESULT_VARIABLE __RES
    )
    if(NOT __RES EQUAL 0)
        file(REMOVE_RECURSE ${VENDOR_DIR}/sdl)
    endif()
endif()
if(NOT IS_DIRECTORY ${VENDOR_DIR}/sdl/build)
    execute_process(
        COMMAND bash -c [[
            set -e
            cmake -B build \
                -DCMAKE_BUILD_TYPE=Release \
                -DSDL_TESTS=OFF
            cmake --build build --config Release
        ]]
        WORKING_DIRECTORY ${VENDOR_DIR}/sdl
        RESULT_VARIABLE __RES
    )
    if(NOT __RES EQUAL 0)
        file(REMOVE_RECURSE ${VENDOR_DIR}/sdl/build)
    endif()
endif()

add_library(SDL STATIC IMPORTED)
set_target_properties(SDL PROPERTIES
    IMPORTED_LOCATION ${VENDOR_DIR}/sdl/build/libSDL3.so
)
target_include_directories(SDL INTERFACE ${VENDOR_DIR}/sdl/include)
