set(SDL3_IMAGE_SOURCE_DIR  ${DEPS_SOURCE_DIR}/SDL_image)
set(SDL3_IMAGE_BUILD_DIR   ${DEPS_BUILD_DIR}/SDL3_image/build)
set(SDL3_IMAGE_INSTALL_DIR ${DEPS_BUILD_DIR}/SDL3_image/install)

if(NOT EXISTS ${SDL3_IMAGE_INSTALL_DIR}/lib/cmake/SDL3_image/SDL3_imageConfig.cmake)
    message(STATUS "SDL3_image pre-built not found, building (one-time)...")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            ${DEPS_CMAKE_CONFIGURE_ARGS}
            --log-level=WARNING
            -S ${SDL3_IMAGE_SOURCE_DIR}
            -B ${SDL3_IMAGE_BUILD_DIR}
            -DCMAKE_BUILD_TYPE=Release
            -DCMAKE_INSTALL_PREFIX=${SDL3_IMAGE_INSTALL_DIR}
            -DSDL3_DIR=${SDL3_INSTALL_DIR}/lib/cmake/SDL3
            -DBUILD_SHARED_LIBS=${DEPS_BUILD_SHARED}
            -DSDLIMAGE_VENDORED=OFF
            -DSDLIMAGE_DEPS_SHARED=${DEPS_BUILD_SHARED}
            # Only PNG, decoded by SDL_image's bundled stb_image (no libpng or extra
            # submodules needed) — the project's texture assets are all .png.
            -DSDLIMAGE_BACKEND_STB=ON
            -DSDLIMAGE_PNG=ON
            -DSDLIMAGE_PNG_LIBPNG=OFF
            -DSDLIMAGE_ANI=OFF
            -DSDLIMAGE_AVIF=OFF
            -DSDLIMAGE_BMP=OFF
            -DSDLIMAGE_GIF=OFF
            -DSDLIMAGE_JPG=OFF
            -DSDLIMAGE_JXL=OFF
            -DSDLIMAGE_LBM=OFF
            -DSDLIMAGE_PCX=OFF
            -DSDLIMAGE_PNM=OFF
            -DSDLIMAGE_QOI=OFF
            -DSDLIMAGE_SVG=OFF
            -DSDLIMAGE_TGA=OFF
            -DSDLIMAGE_TIF=OFF
            -DSDLIMAGE_WEBP=OFF
            -DSDLIMAGE_XCF=OFF
            -DSDLIMAGE_XPM=OFF
            -DSDLIMAGE_XV=OFF
            -DSDLIMAGE_SAMPLES=OFF
            -DSDLIMAGE_TESTS=OFF
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} --build ${SDL3_IMAGE_BUILD_DIR} --config Release --parallel
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} --install ${SDL3_IMAGE_BUILD_DIR} --config Release
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

# NO_DEFAULT_PATH: sadece kendi kurduğumuz yere bak, sistem SDL3_image'ını alma.
set(SDL3_image_DIR "${SDL3_IMAGE_INSTALL_DIR}/lib/cmake/SDL3_image" CACHE PATH
        "Path to the project-built shared SDL3_image package" FORCE)
find_package(SDL3_image REQUIRED PATHS ${SDL3_IMAGE_INSTALL_DIR} NO_DEFAULT_PATH)
