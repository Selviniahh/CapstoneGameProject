set(DEPS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps)

# Desktop builds link the dependencies as shared libraries. A cross build cannot: Emscripten has
# no dynamic linking worth the name, and every target needs its own cache anyway, so those get a
# static build under their own directory (depbuilt/Emscripten-static, depbuilt/Android-static, ...).
if(EMSCRIPTEN OR ANDROID OR CMAKE_CROSSCOMPILING)
    set(DEPS_BUILD_SHARED OFF)
    set(DEPS_BUILD_STATIC ON)
    set(DEPS_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/depbuilt/${CMAKE_SYSTEM_NAME}-static)
else()
    # Keep shared dependency builds separate from older static cached installs.
    set(DEPS_BUILD_SHARED ON)
    set(DEPS_BUILD_STATIC OFF)
    set(DEPS_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/depbuilt/shared)
endif()

# CLion can configure multiple profiles concurrently. Serialize access to the
# shared dependency cache so two CMake processes never configure/build it at
# the same time.
file(MAKE_DIRECTORY "${DEPS_BUILD_DIR}")
file(LOCK "${DEPS_BUILD_DIR}/configure.lock"
        GUARD PROCESS
        TIMEOUT 600
        RESULT_VARIABLE DEPS_LOCK_RESULT)
if(NOT DEPS_LOCK_RESULT STREQUAL "0")
    message(FATAL_ERROR "Could not lock dependency build cache: ${DEPS_LOCK_RESULT}")
endif()

# Use the same generator and toolchain as the main project. This prevents a
# cached dependency build from being reconfigured by a different compiler.
set(DEPS_CMAKE_CONFIGURE_ARGS -G "${CMAKE_GENERATOR}")

if(CMAKE_GENERATOR_PLATFORM)
    list(APPEND DEPS_CMAKE_CONFIGURE_ARGS -A "${CMAKE_GENERATOR_PLATFORM}")
endif()

if(CMAKE_GENERATOR_TOOLSET)
    list(APPEND DEPS_CMAKE_CONFIGURE_ARGS -T "${CMAKE_GENERATOR_TOOLSET}")
endif()

if(CMAKE_TOOLCHAIN_FILE)
    list(APPEND DEPS_CMAKE_CONFIGURE_ARGS
            "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
else()
    list(APPEND DEPS_CMAKE_CONFIGURE_ARGS
            "-DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}"
            "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
endif()
