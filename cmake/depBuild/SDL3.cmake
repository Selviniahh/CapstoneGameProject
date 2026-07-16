set(SDL3_SOURCE_DIR  ${DEPS_SOURCE_DIR}/SDL3)
set(SDL3_BUILD_DIR   ${DEPS_BUILD_DIR}/SDL3/build)
set(SDL3_INSTALL_DIR ${DEPS_BUILD_DIR}/SDL3/install)

if(NOT EXISTS ${SDL3_INSTALL_DIR}/lib/cmake/SDL3/SDL3Config.cmake)
    message(STATUS "SDL3 pre-built not found, building (one-time)...")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            ${DEPS_CMAKE_CONFIGURE_ARGS} #${DEPS_CMAKE_CONFIGURE_ARGS} #It re-passes the parent CMake configure's generator, toolchain, and compilers to the dependency build, so both use the same settings.
            
            --log-level=WARNING
            -S ${SDL3_SOURCE_DIR}
            -B ${SDL3_BUILD_DIR}
            -DCMAKE_BUILD_TYPE=Release
            -DCMAKE_INSTALL_PREFIX=${SDL3_INSTALL_DIR}
            -DSDL_SHARED=ON
            -DSDL_STATIC=OFF
            -DSDL_TEST_LIBRARY=OFF
            -DSDL_TESTS=OFF
            -DSDL_EXAMPLES=OFF
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} --build ${SDL3_BUILD_DIR} --config Release --parallel
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} --install ${SDL3_BUILD_DIR} --config Release
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

# NO_DEFAULT_PATH: sadece kendi kurduğumuz yere bak, sistem SDL3'ünü alma.
#CMake cache değişkenlerinde bazı türler bulunur:
#- BOOL → ON/OFF
#- STRING → normal metin
#- FILEPATH → dosya yolu
#- PATH → klasör yolu
set(SDL3_DIR "${SDL3_INSTALL_DIR}/lib/cmake/SDL3" CACHE PATH
        "Path to the project-built shared SDL3 package" FORCE)

find_package(SDL3 REQUIRED PATHS ${SDL3_INSTALL_DIR} NO_DEFAULT_PATH)
