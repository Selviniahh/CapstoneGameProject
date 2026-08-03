# Compiles src/Engine/Shaders/*.sc into Resources/Shaders/<profile>/<name>.sc.bin with bgfx's
# shaderc, one binary per backend.
#
# The outputs live in the source tree and are committed, because a shader binary is per *backend*,
# not per host: a Linux machine cannot produce D3D bytecode and an Emscripten cross build cannot
# run shaderc at all. Checking them in means a fresh clone (and every cross build) has working
# shaders without any tool, while a normal desktop build with ETG_COMPILE_SHADERS=ON keeps them
# regenerated from source.
#
# GraphicsDevice picks the directory at runtime from bgfx::getRendererType(); the names here match
# the ones bgfx itself uses (see bgfx/examples/runtime/shaders).

set(ETG_SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/Engine/Shaders)
set(ETG_SHADER_OUTPUT_DIR ${CMAKE_SOURCE_DIR}/Resources/Shaders)
set(ETG_SHADER_VARYING_DEF ${ETG_SHADER_SOURCE_DIR}/varying.def.sc)
set(ETG_SHADER_INCLUDE_DIR ${DEPS_SOURCE_DIR}/bgfx.cmake/bgfx/src)

# profile;platform;output directory. Everything except D3D bytecode can be produced from any host:
#   glsl  desktop OpenGL                     essl  WebGL 2 / OpenGL ES 3 (browser, Android, ...)
#   spirv Vulkan                             metal macOS + iOS
#   dxbc  Direct3D 11 (needs fxc: Windows)   dxil  Direct3D 12 (dxcompiler, shipped with bgfx)
# Written with "|" separators so CMake does not flatten them into one long list.
set(ETG_SHADER_TARGETS
        "120|linux|glsl"
        "300_es|android|essl"
        "spirv|linux|spirv"
        "metal|osx|metal"
        "s_6_0|windows|dxil"
)
if(WIN32)
    list(APPEND ETG_SHADER_TARGETS "s_5_0|windows|dxbc")
endif()

# etg_compile_shaders(<target name>)
# Adds a custom target that (re)builds every shader for every profile above.
function(etg_compile_shaders TARGET_NAME)
    set(ALL_OUTPUTS "")

    foreach(SHADER IN ITEMS vs_sprite fs_sprite fs_sprite_grayscale)
        if(SHADER MATCHES "^vs_")
            set(SHADER_TYPE vertex)
        else()
            set(SHADER_TYPE fragment)
        endif()

        foreach(SHADER_TARGET IN LISTS ETG_SHADER_TARGETS)
            string(REPLACE "|" ";" SHADER_TARGET_PARTS ${SHADER_TARGET})
            list(GET SHADER_TARGET_PARTS 0 PROFILE)
            list(GET SHADER_TARGET_PARTS 1 PLATFORM)
            list(GET SHADER_TARGET_PARTS 2 OUT_DIR_NAME)

            set(OUTPUT ${ETG_SHADER_OUTPUT_DIR}/${OUT_DIR_NAME}/${SHADER}.sc.bin)
            add_custom_command(
                    OUTPUT ${OUTPUT}
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${ETG_SHADER_OUTPUT_DIR}/${OUT_DIR_NAME}
                    COMMAND bgfx::shaderc
                            -f ${ETG_SHADER_SOURCE_DIR}/${SHADER}.sc
                            -o ${OUTPUT}
                            --type ${SHADER_TYPE}
                            --platform ${PLATFORM}
                            --profile ${PROFILE}
                            --varyingdef ${ETG_SHADER_VARYING_DEF}
                            -i ${ETG_SHADER_INCLUDE_DIR}
                            -O 3
                    MAIN_DEPENDENCY ${ETG_SHADER_SOURCE_DIR}/${SHADER}.sc
                    DEPENDS ${ETG_SHADER_VARYING_DEF}
                    COMMENT "shaderc ${SHADER}.sc -> ${OUT_DIR_NAME}"
                    VERBATIM
            )
            list(APPEND ALL_OUTPUTS ${OUTPUT})
        endforeach()
    endforeach()

    add_custom_target(${TARGET_NAME} DEPENDS ${ALL_OUTPUTS})
endfunction()
