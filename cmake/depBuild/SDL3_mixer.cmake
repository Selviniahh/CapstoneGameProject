set(SDL3_MIXER_SOURCE_DIR  ${DEPS_SOURCE_DIR}/SDL_mixer)
set(SDL3_MIXER_BUILD_DIR   ${DEPS_BUILD_DIR}/SDL3_mixer/build)
set(SDL3_MIXER_INSTALL_DIR ${DEPS_BUILD_DIR}/SDL3_mixer/install)

#OGG         → Sesin bulunduğu container
#Vorbis      → Ses codec’i
#stb_vorbis  → Vorbis sesini decode eden küçük kütüphane
#- libvorbisfile: Resmî, daha kapsamlı; libogg ve libvorbis gibi
#    ek bağımlılıkları var. Bunu kullanmiyoruz

if(NOT EXISTS ${SDL3_MIXER_INSTALL_DIR}/lib/cmake/SDL3_mixer/SDL3_mixerConfig.cmake)
    message(STATUS "SDL3_mixer pre-built not found, building (one-time)...")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            ${DEPS_CMAKE_CONFIGURE_ARGS}
            --log-level=WARNING
            -S ${SDL3_MIXER_SOURCE_DIR}
            -B ${SDL3_MIXER_BUILD_DIR}
            -DCMAKE_BUILD_TYPE=Release
            -DCMAKE_INSTALL_PREFIX=${SDL3_MIXER_INSTALL_DIR}
            -DSDL3_DIR=${SDL3_INSTALL_DIR}/lib/cmake/SDL3
            -DBUILD_SHARED_LIBS=${DEPS_BUILD_SHARED}
            -DSDLMIXER_VENDORED=OFF
            -DSDLMIXER_DEPS_SHARED=${DEPS_BUILD_SHARED}
            # Only WAVE + OGG (via SDL_mixer's own bundled stb_vorbis, no extra submodules
            # needed) — the project's sound assets are all .ogg.
            -DSDLMIXER_WAVE=ON
            -DSDLMIXER_VORBIS_STB=ON
            -DSDLMIXER_VORBIS_VORBISFILE=OFF
            -DSDLMIXER_VORBIS_TREMOR=OFF
            -DSDLMIXER_MP3=OFF
            -DSDLMIXER_FLAC=OFF
            -DSDLMIXER_OPUS=OFF
            -DSDLMIXER_MOD=OFF
            -DSDLMIXER_MIDI=OFF
            -DSDLMIXER_GME=OFF
            -DSDLMIXER_WAVPACK=OFF
            -DSDLMIXER_AIFF=OFF
            -DSDLMIXER_VOC=OFF
            -DSDLMIXER_AU=OFF
            -DSDLMIXER_TESTS=OFF
            -DSDLMIXER_EXAMPLES=OFF
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} --build ${SDL3_MIXER_BUILD_DIR} --config Release --parallel
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} --install ${SDL3_MIXER_BUILD_DIR} --config Release
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

# NO_DEFAULT_PATH: sadece kendi kurduğumuz yere bak, sistem SDL3_mixer'ını alma.
set(SDL3_mixer_DIR "${SDL3_MIXER_INSTALL_DIR}/lib/cmake/SDL3_mixer" CACHE PATH
        "Path to the project-built shared SDL3_mixer package" FORCE)
find_package(SDL3_mixer REQUIRED PATHS ${SDL3_MIXER_INSTALL_DIR} NO_DEFAULT_PATH)
