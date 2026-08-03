# bgfx (+ bx, bimg) — the renderer. Built from the bkaradzic/bgfx.cmake submodule under
# deps/bgfx.cmake (which carries bgfx/bx/bimg as its own submodules), cached in depbuilt/ exactly
# like the SDL family above it.
#
# Static on purpose: bgfx exports no dllexport'ed C++ API, so a shared build would need
# BGFX_SHARED_LIB_BUILD plumbing on Windows for no benefit. It is compiled with -fPIC so it can
# still be linked into the project's own shared libraries.
set(BGFX_SOURCE_DIR  ${DEPS_SOURCE_DIR}/bgfx.cmake)
set(BGFX_BUILD_DIR   ${DEPS_BUILD_DIR}/bgfx/build)
set(BGFX_INSTALL_DIR ${DEPS_BUILD_DIR}/bgfx/install)

# shaderc is a *host* tool. When cross compiling (Emscripten, Android) it would be built for the
# target and be unrunnable, so those builds use the shader binaries checked into Resources/Shaders.
if(CMAKE_CROSSCOMPILING OR EMSCRIPTEN OR ANDROID)
    set(ETG_SHADER_TOOLS_DEFAULT OFF)
else()
    set(ETG_SHADER_TOOLS_DEFAULT ON)
endif()

option(ETG_COMPILE_SHADERS
        "Recompile src/Engine/Shaders/*.sc into Resources/Shaders with bgfx's shaderc during the build"
        ${ETG_SHADER_TOOLS_DEFAULT})

if(ETG_COMPILE_SHADERS)
    set(BGFX_TOOLS_ARG ON)
else()
    set(BGFX_TOOLS_ARG OFF)
endif()

# Emscripten advertises itself as an x86 platform, which makes bgfx.cmake compile bx (and
# everything linking it) with -msse4.2 and drags in x86-only intrinsics that clang has no wasm
# lowering for. The toolchain file lets that be overridden, and telling it the truth is enough for
# bx and bimg to take their portable paths.
set(BGFX_EXTRA_ARGS "")
if(EMSCRIPTEN)
    list(APPEND BGFX_EXTRA_ARGS "-DEMSCRIPTEN_SYSTEM_PROCESSOR=wasm32")
endif()

# A cached build made without the tools can't satisfy a later ETG_COMPILE_SHADERS=ON, so the
# marker checked here is shaderc itself whenever the tools are wanted.
set(BGFX_CACHE_MARKER ${BGFX_INSTALL_DIR}/lib/cmake/bgfx/bgfxConfig.cmake)
if(ETG_COMPILE_SHADERS AND NOT EXISTS ${BGFX_INSTALL_DIR}/bin/shaderc${CMAKE_EXECUTABLE_SUFFIX})
    set(BGFX_CACHE_MARKER ${BGFX_INSTALL_DIR}/bin/shaderc${CMAKE_EXECUTABLE_SUFFIX})
endif()

if(NOT EXISTS ${BGFX_CACHE_MARKER})
    message(STATUS "bgfx pre-built not found, building (one-time)...")
    execute_process(
        COMMAND ${CMAKE_COMMAND}
            ${DEPS_CMAKE_CONFIGURE_ARGS}
            --log-level=WARNING
            -S ${BGFX_SOURCE_DIR}
            -B ${BGFX_BUILD_DIR}
            -DCMAKE_BUILD_TYPE=Release
            -DCMAKE_INSTALL_PREFIX=${BGFX_INSTALL_DIR}
            -DCMAKE_POSITION_INDEPENDENT_CODE=ON
            -DBGFX_LIBRARY_TYPE=STATIC
            -DBGFX_INSTALL=ON
            -DBGFX_BUILD_EXAMPLES=OFF
            -DBGFX_BUILD_TESTS=OFF
            -DBGFX_BUILD_TOOLS=${BGFX_TOOLS_ARG}
            -DBGFX_BUILD_TOOLS_SHADER=${BGFX_TOOLS_ARG}
            -DBGFX_BUILD_TOOLS_BIN2C=OFF
            -DBGFX_BUILD_TOOLS_GEOMETRY=OFF
            -DBGFX_BUILD_TOOLS_TEXTURE=OFF
            ${BGFX_EXTRA_ARGS}
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} --build ${BGFX_BUILD_DIR} --config Release --parallel
        COMMAND_ERROR_IS_FATAL ANY
    )
    execute_process(
        COMMAND ${CMAKE_COMMAND} --install ${BGFX_BUILD_DIR} --config Release
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

set(bgfx_DIR "${BGFX_INSTALL_DIR}/lib/cmake/bgfx" CACHE PATH
        "Path to the project-built bgfx package" FORCE)
find_package(bgfx REQUIRED PATHS ${BGFX_INSTALL_DIR} NO_DEFAULT_PATH)
