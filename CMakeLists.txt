cmake_minimum_required(VERSION 3.28)

project(ETG LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build the project's compiled libraries as shared libraries by default.
option(BUILD_SHARED_LIBS "Build project libraries as shared libraries" ON)

# Export symbols automatically from our DLL targets on Windows. This keeps the
# current source code portable without requiring __declspec(dllexport) macros.
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

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
#  - Dear ImGui (+ SDL3 backends): git submodule under deps/imgui
#  - Boost (header-only: type_index, describe): required Boost modules are git
#    submodules under deps/boost
# ------------------------------------------------------------------------------
include(cmake/DepConfig.cmake)
include(cmake/depBuild/SDL3.cmake)
include(cmake/depBuild/SDL3_mixer.cmake) # SDL3'ten SONRA: kendi find_package(SDL3) çağrısı için SDL3_DIR gerekiyor
include(cmake/depBuild/SDL3_image.cmake) # SDL3'ten SONRA
include(cmake/depBuild/SDL3_ttf.cmake)   # SDL3'ten SONRA
include(cmake/depBuild/imgui.cmake)   # SDL3'ten SONRA: imgui SDL3::SDL3'e link eder
include(cmake/depBuild/boost.cmake)

#std::thread icin 
find_package(Threads REQUIRED)

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------
set(PLATFORM_SOURCES
        src/Platform/Vector2.h
        src/Platform/Rect.h
        src/Platform/Color.h
        src/Platform/View.h
        src/Platform/Texture.cpp
        src/Platform/Texture.h
        src/Platform/Sprite.h
        src/Platform/RenderWindow.cpp
        src/Platform/RenderWindow.h
        src/Platform/Shapes.h
        src/Platform/Input.cpp
        src/Platform/Input.h
        src/Platform/Audio.cpp
        src/Platform/Audio.h
        src/Platform/Text.cpp
        src/Platform/Text.h
        src/Platform/Platform.h
)

set(CORE_SOURCES
        src/Core/GameObjectBase.cpp
        src/Core/GameObjectBase.h
        src/Core/ComponentBase.h
        src/Core/Factory.h
        src/Core/GameClass.cpp
        src/Core/GameClass.h
        src/Core/TypeID.h
        src/Core/Scene/Scene.cpp
        src/Core/Scene/Scene.h
        src/Core/Components/BaseHealthComp.cpp
        src/Core/Components/BaseHealthComp.h
        src/Core/Components/BaseMoveComp.cpp
        src/Core/Components/BaseMoveComp.h
        src/Core/Components/BaseAnimComp.h
        src/Core/Components/ArrowComp.h
        src/Core/Components/CollisionComponent.cpp
        src/Core/Components/CollisionComponent.h
        src/Core/Components/TimerComponent.cpp
        src/Core/Components/TimerComponent.h
        src/Core/Events/EventDelegate.h
)

set(ENGINE_SOURCES
        src/Engine/Engine.cpp
        src/Engine/Engine.h
        src/Engine/Reflection.h
        src/Engine/UI/EngineUI.cpp
        src/Engine/UI/EngineUI.h
        src/Engine/UI/UIUtils.cpp
        src/Engine/UI/UIUtils.h
)

set(UI_SOURCES
        src/UI/UserInterface.cpp
        src/UI/UIObjects/AmmoBarUI.cpp
        src/UI/UIObjects/AmmoBarUI.h
        src/UI/UIObjects/AmmoIndicatorsUI.cpp
        src/UI/UIObjects/AmmoIndicatorsUI.h
        src/UI/UIObjects/ReloadText.h
        src/UI/UIObjects/AmmoCounter.h
        src/UI/UIObjects/ReloadSlider.cpp
        src/UI/UIObjects/ReloadSlider.h
        src/UI/UIObjects/FrameBar.h
        src/UI/UIObjects/FrameBar.cpp
        src/UI/UIObjects/FrameLeftProgressBar.cpp
        src/UI/UIObjects/FrameLeftProgressBar.h
)

set(MANAGERS_SOURCES
        src/Managers/AssetManager.cpp
        src/Managers/AssetManager.h
        src/Managers/Globals.cpp
        src/Managers/GameManager.cpp
        src/Managers/GameState.h
        src/Managers/SpriteBatch.cpp
        src/Managers/InputManager.cpp
        src/Managers/TypeRegistry.h
        src/Managers/Enum/StateFlags.h
)

set(ANIMATION_SOURCES
        src/Animation/Animation.cpp
)

set(CHARACTERS_SOURCES
        src/Characters/Hero.cpp
        src/Characters/Components/HeroAnimComp.cpp
        src/Characters/Components/InputComponent.cpp
        src/Characters/Components/HeroMoveComp.cpp
        src/Characters/Hand/Hand.cpp
        src/Characters/Hand/Hand.h
)

set(GUNS_SOURCES
        src/Guns/Base/GunBase.cpp
        src/Guns/Base/GunBase.h
        src/Guns/RogueSpecial/RogueSpecial.cpp
        src/Guns/RogueSpecial/RogueSpecial.h
        src/Guns/VFX/MuzzleFlash.cpp
        src/Guns/VFX/MuzzleFlash.h
        src/Guns/AK-47/AK47.cpp
        src/Guns/AK-47/AK47.h
        src/Guns/SawedOff/SawedOff.cpp
        src/Guns/SawedOff/SawedOff.h
        src/Guns/Magnum/Magnum.cpp
        src/Guns/Magnum/Magnum.h
)

set(PROJECTILE_SOURCES
        src/Projectile/ProjectileBase.cpp
        src/Projectile/ProjectileBase.h
)

set(ENEMY_SOURCES
        src/Enemy/EnemyBase.cpp
        src/Enemy/EnemyBase.h
        src/Enemy/BulletMan/BulletMan.cpp
        src/Enemy/BulletMan/BulletMan.h
        src/Enemy/BulletMan/Components/BulletManAnimComp.cpp
        src/Enemy/BulletMan/Components/BulletManAnimComp.h
        src/Enemy/Components/EnemyMoveCompBase.cpp
        src/Enemy/Components/EnemyMoveCompBase.h
)

set(ITEMS_SOURCES
        src/Items/Active/ActiveItemBase.h
        src/Items/Active/ActiveItemBase.cpp
        src/Items/Active/DoubleShoot.cpp
        src/Items/Active/DoubleShoot.h
        src/Items/Passive/PassiveItemBase.h
        src/Items/Passive/PlatinumBullets.cpp
        src/Items/Passive/PlatinumBullets.h
)

set(MODIFIERS_SOURCES
        src/Modifiers/ModifierManager.h
        src/Modifiers/Gun/MultiShotModifier.h
)

set(UTILS_SOURCES
        src/Utils/DirectionUtils.cpp
        src/Utils/DirectionUtils.h
        src/Utils/Math.h
        src/Utils/TextureUtils.h
        src/Utils/StrManipulateUtil.h
        src/Utils/Interface/IAnimationComponent.h
        src/Utils/Interface/IGunModifier.h
)

# Create a master library for all SDL-dependent code.
# Its type is controlled by BUILD_SHARED_LIBS (shared by default).
add_library(etgcore
        ${PLATFORM_SOURCES}
        ${CORE_SOURCES}
        ${ENGINE_SOURCES}
        ${ANIMATION_SOURCES}
        ${MANAGERS_SOURCES}
        ${CHARACTERS_SOURCES}
        ${GUNS_SOURCES}
        ${UI_SOURCES}
        ${ENEMY_SOURCES}
        ${PROJECTILE_SOURCES}
        ${ITEMS_SOURCES}
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
        Boost::type_index
        Boost::describe
)

add_library(utils ${UTILS_SOURCES})
target_include_directories(utils PUBLIC src)
target_link_libraries(utils PUBLIC Boost::type_index Boost::describe SDL3::SDL3 imgui etg_compile_flags)

add_library(modifiers INTERFACE)
target_sources(modifiers INTERFACE ${MODIFIERS_SOURCES})
target_include_directories(modifiers INTERFACE src)

add_executable(ETG main.cpp)

target_link_libraries(ETG PRIVATE
        etgcore  # Our consolidated core library
        utils
        modifiers
        SDL3::SDL3
        SDL3_mixer::SDL3_mixer
        imgui
        Boost::type_index
        Boost::describe
        Threads::Threads
)

# AssetManager resolves assets relative to the executable (SDL_GetBasePath), so
# mirror the Resources folder next to the binary after every successful build.
add_custom_command(TARGET ETG POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                ${CMAKE_SOURCE_DIR}/Resources
                $<TARGET_FILE_DIR:ETG>/Resources
        COMMENT "Copying Resources next to ETG"
)

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
