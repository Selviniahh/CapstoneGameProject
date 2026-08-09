cmake_minimum_required(VERSION 3.28)

project(ETG LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build the project's compiled libraries as shared libraries by default.
option(BUILD_SHARED_LIBS "Build project libraries as shared libraries" ON)

# Emscripten links everything into one wasm module; shared libraries there mean side modules,
# which buy nothing for a single executable.
if(EMSCRIPTEN)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build project libraries as shared libraries" FORCE)
endif()

# Export symbols automatically from our DLL targets on Windows. This keeps the
# current source code portable without requiring __declspec(dllexport) macros.
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# ------------------------------------------------------------------------------
# Tests (Test/). Two suites, two executables, and two switches deciding whether the
# game's build waits on them. Everything here is off/harmless by default: with no
# option touched, the game builds and runs exactly as it did before, and the two test
# executables are simply available to run on their own.
#
#   cmake -B build                                                the game, both test targets built, no gates
#   cmake -B build -DETG_RUN_UNIT_TESTS_BEFORE_GAME=ON            unit tests must pass before the game builds
#   cmake -B build -DETG_RUN_INTERACTIVE_TESTS_BEFORE_GAME=ON     the interactive session opens before the game
#   cmake -B build -DETG_BUILD_UNIT_TESTS=OFF -DETG_BUILD_INTERACTIVE_TESTS=OFF   game only, nothing else built
#
# Test/README.md walks through the combinations and how to write a new test.
# ------------------------------------------------------------------------------
option(ETG_BUILD_UNIT_TESTS "Build the GoogleTest unit test target (ETGUnitTests)" ON)
option(ETG_BUILD_INTERACTIVE_TESTS "Build the interactive gameplay test target (ETGInteractiveTests)" ON)
option(ETG_RUN_UNIT_TESTS_BEFORE_GAME "Run ETGUnitTests as part of building ETG, before the game can start" OFF)
option(ETG_RUN_INTERACTIVE_TESTS_BEFORE_GAME "Launch ETGInteractiveTests as part of building ETG, before the game can start" OFF)
option(ETG_UNIT_TESTS_FAIL_BUILD "A failing unit test gate stops the build" ON)
option(ETG_INTERACTIVE_TESTS_FAIL_BUILD "A failed interactive gameplay check stops the build" ON)
option(ETG_INTERACTIVE_TESTS_STRICT "The interactive gate also demands that every test was played to a verdict" OFF)

#- CMAKE_INSTALL_BINDIR → usually in "bin"
#CMAKE_INSTALL_LIBDIR -> usually "lib"  
include(GNUInstallDirs)

# Places Statik libs under (.a, .lib) build/lib.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

#Linux’taki paylaşımlı kütüphaneleri (.so) ve macOS’taki .dylib dosyalarını build/lib altına koyar.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})

#Çalıştırılabilir dosyaları ve Windows’taki .dll dosyalarını  build/bin altına koyar.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

# ------------------------------------------------------------------------------
# Dependencies (no package manager required, everything built from source)
#  - SDL3: git submodule under deps/SDL3, built once and cached in depbuilt/
#  - SDL3_mixer (audio decoding/playback): git submodule under deps/SDL_mixer, built once
#    and cached in depbuilt/ (needs SDL3 built first)
#  - SDL3_image (PNG decoding): git submodule under deps/SDL_image, built once
#    and cached in depbuilt/ (needs SDL3 built first)
#  - SDL3_ttf (font rendering, vendored freetype): git submodule under deps/SDL_ttf,
#    built once and cached in depbuilt/ (needs SDL3 built first)
#  - bgfx (+ bx, bimg): the renderer. git submodule under deps/bgfx.cmake, built once and cached
#    in depbuilt/. Also provides shaderc, which compiles src/Engine/Shaders into Resources/Shaders
#  - Dear ImGui (+ SDL3 platform backend; the renderer backend is ours, see ImGuiBgfxBackend)
#  - Boost (header-only: type_index, describe): required Boost modules are git
#    submodules under deps/boost
# ------------------------------------------------------------------------------
include(cmake/DepConfig.cmake)
include(cmake/depBuild/SDL3.cmake)
include(cmake/depBuild/SDL3_mixer.cmake) # SDL3'ten SONRA: kendi find_package(SDL3) çağrısı için SDL3_DIR gerekiyor
include(cmake/depBuild/SDL3_image.cmake) # SDL3'ten SONRA
include(cmake/depBuild/SDL3_ttf.cmake)   # SDL3'ten SONRA
include(cmake/depBuild/imgui.cmake)   # SDL3'ten SONRA: imgui SDL3::SDL3'e link eder
include(cmake/depBuild/bgfx.cmake)
include(cmake/depBuild/boost.cmake)
include(cmake/Shaders.cmake)

#std::thread icin 
find_package(Threads REQUIRED)

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------
set(PLATFORM_SOURCES
        src/Engine/Platform/Vector2.h
        src/Engine/Platform/Rect.h
        src/Engine/Platform/Color.h
        src/Engine/Platform/View.h
        src/Engine/Platform/GraphicsDevice.cpp
        src/Engine/Platform/GraphicsDevice.h
        src/Engine/Platform/Texture.cpp
        src/Engine/Platform/Texture.h
        src/Engine/Platform/Sprite.h
        src/Engine/Platform/RenderWindow.cpp
        src/Engine/Platform/RenderWindow.h
        src/Engine/Platform/Shapes.h
        src/Engine/Platform/Input.cpp
        src/Engine/Platform/Input.h
        src/Engine/Platform/Audio.cpp
        src/Engine/Platform/Audio.h
        src/Engine/Platform/Text.cpp
        src/Engine/Platform/Text.h
        src/Engine/Platform/Platform.h
)

set(CORE_SOURCES
        src/Engine/Core/GameObjectBase.cpp
        src/Engine/Core/GameObjectBase.h
        src/Engine/Core/ComponentBase.h
        src/Engine/Core/Factory.h
        src/Engine/Core/GameClass.cpp
        src/Engine/Core/GameClass.h
        src/Engine/Core/SingleInstance.h
        src/Engine/Core/TypeID.h
        src/Engine/Core/Direction.h
        src/Engine/Core/Scene/Scene.cpp
        src/Engine/Core/Scene/Scene.h
        src/Engine/Core/Components/BaseHealthComp.cpp
        src/Engine/Core/Components/BaseHealthComp.h
        src/Engine/Core/Components/BaseMoveComp.cpp
        src/Engine/Core/Components/BaseMoveComp.h
        src/Engine/Core/Components/BaseAnimComp.h
        src/Engine/Core/Components/ArrowComp.h
        src/Engine/Core/Components/CollisionComponent.cpp
        src/Engine/Core/Components/CollisionComponent.h
        src/Engine/Core/Components/TimerComponent.cpp
        src/Engine/Core/Components/TimerComponent.h
        src/Engine/Core/Events/EventDelegate.h
        src/Engine/Core/StateMachine/StateNode.h
        src/Engine/Core/StateMachine/HierarchicalStateMachine.h
        src/Engine/Core/Stats/StatModifier.cpp
        src/Engine/Core/Stats/StatModifier.h
)

set(ENGINE_SOURCES
        src/Engine/Editor/Engine.cpp
        src/Engine/Editor/Engine.h
        src/Engine/Editor/Reflection.h
        src/Engine/Editor/DebugPointViz.cpp
        src/Engine/Editor/DebugPointViz.h
        src/Engine/Editor/UI/EngineUI.cpp
        src/Engine/Editor/UI/EngineUI.h
        src/Engine/Editor/UI/UIUtils.cpp
        src/Engine/Editor/UI/UIUtils.h
        src/Engine/Editor/UI/ImGuiBgfxBackend.cpp
        src/Engine/Editor/UI/ImGuiBgfxBackend.h
)

set(SHADER_SOURCES
        src/Engine/Shaders/varying.def.sc
        src/Engine/Shaders/vs_sprite.sc
        src/Engine/Shaders/fs_sprite.sc
        src/Engine/Shaders/fs_sprite_grayscale.sc
)

set(UI_SOURCES
        src/Game/UI/UserInterface.cpp
        src/Game/UI/UIObjects/AmmoBarUI.cpp
        src/Game/UI/UIObjects/AmmoBarUI.h
        src/Game/UI/UIObjects/AmmoIndicatorsUI.cpp
        src/Game/UI/UIObjects/AmmoIndicatorsUI.h
        src/Game/UI/UIObjects/ReloadText.h
        src/Game/UI/UIObjects/AmmoCounter.h
        src/Game/UI/UIObjects/ReloadSlider.cpp
        src/Game/UI/UIObjects/ReloadSlider.h
        src/Game/UI/UIObjects/FrameBar.h
        src/Game/UI/UIObjects/FrameBar.cpp
        src/Game/UI/UIObjects/FrameLeftProgressBar.cpp
        src/Game/UI/UIObjects/FrameLeftProgressBar.h
)

set(MANAGERS_SOURCES
        src/Engine/Managers/AssetManager.cpp
        src/Engine/Managers/AssetManager.h
        src/Engine/Managers/RenderContext.cpp
        src/Engine/Managers/Time.cpp
        src/Engine/Managers/Time.h
        src/Game/Managers/GameManager.cpp
        src/Game/Managers/RegisterGameTypes.cpp
        src/Game/Managers/RegisterGameTypes.h
        src/Engine/Managers/SpriteBatch.cpp
        src/Engine/Managers/InputManager.cpp
        src/Engine/Managers/TypeRegistry.h
        src/Game/Managers/Enum/StateFlags.h
        src/Game/Managers/Enum/FlagOperators.h
        src/Game/Characters/Hero/HeroCapability.h
)

set(LEVELS_SOURCES
        src/Game/Levels/SpawnInitialLevel.cpp
        src/Game/Levels/SpawnInitialLevel.h
)

set(ANIMATION_SOURCES
        src/Engine/Animation/Animation.cpp
        src/Engine/Animation/IAnimationComponent.h
)

set(CHARACTERS_SOURCES
        src/Game/Characters/Hero/Hero.cpp
        src/Game/Characters/Hero/HeroStates.h
        src/Game/Characters/Hero/HeroDirections.cpp
        src/Game/Characters/Hero/HeroDirections.h
        src/Game/Characters/Hero/HeroStateMachine.cpp
        src/Game/Characters/Hero/HeroStateMachine.h
        src/Game/Characters/Hero/Components/HeroAnimComp.cpp
        src/Game/Characters/Hero/Components/InputComponent.cpp
        src/Game/Characters/Hero/Components/HeroMoveComp.cpp
        src/Game/Characters/Hero/Hand/Hand.cpp
        src/Game/Characters/Hero/Hand/Hand.h
        src/Game/Characters/Character.cpp
        src/Game/Characters/Character.h
)

set(GUNS_SOURCES
        src/Game/Guns/GunStates.h
        src/Game/Guns/Base/GunBase.cpp
        src/Game/Guns/Base/HandRig.cpp
        src/Game/Guns/VFX/ShellEjector.cpp
        src/Game/Guns/VFX/ShellEjector.h
        src/Game/Guns/Base/HandRig.h
        src/Game/Guns/Base/GunBase.h
        src/Game/Guns/RogueSpecial/RogueSpecial.cpp
        src/Game/Guns/RogueSpecial/RogueSpecial.h
        src/Game/Guns/VFX/MuzzleFlash.cpp
        src/Game/Guns/VFX/MuzzleFlash.h
        src/Game/Guns/VFX/MagazineDrop.cpp
        src/Game/Guns/VFX/MagazineDrop.h
        src/Game/Guns/AK-47/AK47.cpp
        src/Game/Guns/AK-47/AK47.h
        src/Game/Guns/SawedOff/SawedOff.cpp
        src/Game/Guns/SawedOff/SawedOff.h
        src/Game/Guns/Magnum/Magnum.cpp
        src/Game/Guns/Magnum/Magnum.h
)

set(PROJECTILE_SOURCES
        src/Game/Projectile/ProjectileBase.cpp
        src/Game/Projectile/ProjectileBase.h
)

set(ENEMY_SOURCES
        src/Game/Characters/Enemy/EnemyBase.cpp
        src/Game/Characters/Enemy/EnemyStates.h
        src/Game/Characters/Enemy/BulletMan/BulletManStates.h
        src/Game/Characters/Enemy/BulletMan/BulletManDirections.cpp
        src/Game/Characters/Enemy/BulletMan/BulletManDirections.h
        src/Game/Characters/Enemy/EnemyBase.h
        src/Game/Characters/Enemy/BulletMan/BulletMan.cpp
        src/Game/Characters/Enemy/BulletMan/BulletMan.h
        src/Game/Characters/Enemy/BulletMan/Components/BulletManAnimComp.cpp
        src/Game/Characters/Enemy/BulletMan/Components/BulletManAnimComp.h
        src/Game/Characters/Enemy/Components/EnemyMoveCompBase.cpp
        src/Game/Characters/Enemy/Components/EnemyMoveCompBase.h
)

set(ITEMS_SOURCES
        src/Game/Items/Active/ActiveItemBase.h
        src/Game/Items/Active/ActiveItemBase.cpp
        src/Game/Items/Active/DoubleShoot.cpp
        src/Game/Items/Active/DoubleShoot.h
        src/Game/Items/Passive/PassiveItemBase.h
        src/Game/Items/Passive/PassiveItemBase.cpp
        src/Game/Items/Passive/PlatinumBullets.cpp
        src/Game/Items/Passive/PlatinumBullets.h
        src/Game/Items/Active/TakeNoDamage.cpp
        src/Game/Items/Active/TakeNoDamage.h
)

set(MODIFIERS_SOURCES
        src/Game/Modifiers/ModifierManager.h
        src/Game/Modifiers/Gun/IGunModifier.h
        src/Game/Modifiers/Hero/IHeroModifier.h
)

set(UTILS_SOURCES
        src/Utils/DirectionUtils.cpp
        src/Utils/DirectionUtils.h
        src/Utils/Math.h
        src/Utils/TextureUtils.h
        src/Utils/StrManipulateUtil.h
)

# Create a master library for all SDL-dependent code.
# Its type is controlled by BUILD_SHARED_LIBS (shared by default).
add_library(etgcore
        ${PLATFORM_SOURCES}
        ${CORE_SOURCES}
        ${ENGINE_SOURCES}
        ${ANIMATION_SOURCES}
        ${MANAGERS_SOURCES}
        ${LEVELS_SOURCES}
        ${CHARACTERS_SOURCES}
        ${GUNS_SOURCES}
        ${UI_SOURCES}
        ${ENEMY_SOURCES}
        ${PROJECTILE_SOURCES}
        ${ITEMS_SOURCES}
        ${MODIFIERS_SOURCES}
)

# The codebase uses members that shadow their own type names (e.g. `Engine* Engine;`),
# which GCC rejects by default while MSVC/Clang accept it.
add_library(etg_compile_flags INTERFACE)
target_compile_options(etg_compile_flags INTERFACE
        $<$<CXX_COMPILER_ID:GNU>:-Wno-changes-meaning>
)
target_link_libraries(etgcore PUBLIC etg_compile_flags)

# Include directories
target_include_directories(etgcore PUBLIC src)

# Link the core library's dependencies
target_link_libraries(etgcore PUBLIC
        SDL3::SDL3
        SDL3_mixer::SDL3_mixer
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
        imgui
        bgfx::bgfx
        bgfx::bx
        bgfx::bimg
        Boost::type_index
        Boost::describe
)

add_library(utils ${UTILS_SOURCES})
target_include_directories(utils PUBLIC src)
target_link_libraries(utils PUBLIC Boost::type_index Boost::describe SDL3::SDL3 imgui etg_compile_flags)

# NOTE: the modifier sources are compiled into etgcore, not listed here. A modifier that carries behaviour needs a
# .cpp, and an INTERFACE target would compile that .cpp a second time into every consumer. What is left of this
# target is the include path
add_library(modifiers INTERFACE)
target_include_directories(modifiers INTERFACE src)

add_executable(ETG main.cpp ${SHADER_SOURCES})

# Shader binaries are committed under Resources/Shaders (one directory per backend), so a build
# without the tools still runs. When shaderc is available they are regenerated from the .sc sources
# before ETG links.
if(ETG_COMPILE_SHADERS)
    etg_compile_shaders(etg_shaders)
    add_dependencies(ETG etg_shaders)
endif()

target_link_libraries(ETG PRIVATE
        etgcore  # Our consolidated core library
        utils
        modifiers
        SDL3::SDL3
        SDL3_mixer::SDL3_mixer
        imgui
        bgfx::bgfx
        Boost::type_index
        Boost::describe
        Threads::Threads
)

# ------------------------------------------------------------------------------
# Web build (Emscripten). Configure with the emsdk toolchain file and build as usual:
#   emcmake cmake -S . -B build-web -DCMAKE_BUILD_TYPE=Release
#   cmake --build build-web
# The result is build/bin/ETG.html + .js + .wasm + .data, served over http (file:// won't do).
# ------------------------------------------------------------------------------
if(EMSCRIPTEN)
    set_target_properties(ETG PROPERTIES SUFFIX ".html")
    target_link_options(ETG PRIVATE
            # bgfx renders through WebGL 2 in the browser (the essl shader binaries)
            -sMIN_WEBGL_VERSION=2
            -sMAX_WEBGL_VERSION=2
            -sFULL_ES3=1
            -sALLOW_MEMORY_GROWTH=1
            -sSTACK_SIZE=1MB
            -sEXIT_RUNTIME=0
            # AssetManager reads plain paths, so the whole Resources tree is packaged into MEMFS
            "--preload-file=${CMAKE_SOURCE_DIR}/Resources@/Resources"
    )
endif()

# AssetManager resolves assets relative to the executable (SDL_GetBasePath), so
# mirror the Resources folder next to the binary after every successful build.
add_custom_command(TARGET ETG POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                ${CMAKE_SOURCE_DIR}/Resources
                $<TARGET_FILE_DIR:ETG>/Resources
        COMMENT "Copying Resources next to ETG"
)

# The test targets and the optional "run before the game" gates. Added AFTER the ETG target exists, because a
# gate attaches itself to it with add_dependencies.
enable_testing()
add_subdirectory(Test)

# Windows only searches a limited set of locations for runtime DLLs. Copy all
# shared-library dependencies next to ETG.exe after every successful build.
if(WIN32)
    add_custom_command(TARGET ETG POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_RUNTIME_DLLS:ETG>
                    $<TARGET_FILE_DIR:ETG>
            COMMAND_EXPAND_LISTS
            COMMENT "Copying runtime DLLs next to ETG"
    )
endif()
