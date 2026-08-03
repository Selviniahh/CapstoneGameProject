set(SDL3_TTF_SOURCE_DIR  ${DEPS_SOURCE_DIR}/SDL_ttf)
set(SDL3_TTF_BUILD_DIR   ${DEPS_BUILD_DIR}/SDL3_ttf/build)
set(SDL3_TTF_INSTALL_DIR ${DEPS_BUILD_DIR}/SDL3_ttf/install)

#FreeType — Asıl font dosyasını (.ttf/.otf) okuyup her harfin şeklini (glyph outline'ını) piksellere çeviren kütüphane.
#SDL_ttf'in temel motoru bu; olmazsa hiçbir yazı ekrana basılamaz. Vazgeçilmez

if(NOT EXISTS ${SDL3_TTF_INSTALL_DIR}/lib/cmake/SDL3_ttf/SDL3_ttfConfig.cmake)
    message(STATUS "SDL3_ttf pre-built not found, building (one-time)...")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            ${DEPS_CMAKE_CONFIGURE_ARGS}
            --log-level=WARNING
            -S ${SDL3_TTF_SOURCE_DIR}
            -B ${SDL3_TTF_BUILD_DIR}
            -DCMAKE_BUILD_TYPE=Release
            -DCMAKE_INSTALL_PREFIX=${SDL3_TTF_INSTALL_DIR}
            -DSDL3_DIR=${SDL3_INSTALL_DIR}/lib/cmake/SDL3
            -DBUILD_SHARED_LIBS=${DEPS_BUILD_SHARED}
            # Vendored freetype (deps/SDL_ttf/external/freetype submodule), no system dependency.
            -DSDLTTF_VENDORED=ON
            # ASCII debug/UI text only: no complex shaping (harfbuzz) or color emoji (plutosvg) needed.
            -DSDLTTF_HARFBUZZ=OFF
            -DSDLTTF_PLUTOSVG=OFF
            -DSDLTTF_SAMPLES=OFF
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} --build ${SDL3_TTF_BUILD_DIR} --config Release --parallel
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} --install ${SDL3_TTF_BUILD_DIR} --config Release
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

# NO_DEFAULT_PATH: sadece kendi kurduğumuz yere bak, sistem SDL3_ttf'ini alma.
set(SDL3_ttf_DIR "${SDL3_TTF_INSTALL_DIR}/lib/cmake/SDL3_ttf" CACHE PATH
        "Path to the project-built shared SDL3_ttf package" FORCE)
find_package(SDL3_ttf REQUIRED PATHS ${SDL3_TTF_INSTALL_DIR} NO_DEFAULT_PATH)
