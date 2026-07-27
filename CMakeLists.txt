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
        src/Engine/Platform/Vector2.h
        src/Engine/Platform/Rect.h
        src/Engine/Platform/Color.h
        src/Engine/Platform/View.h
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
        src/Engine/Editor/UI/EngineUI.cpp
        src/Engine/Editor/UI/EngineUI.h
        src/Engine/Editor/UI/UIUtils.cpp
        src/Engine/Editor/UI/UIUtils.h
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
        src/Game/Characters/HeroCapability.h
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
        src/Game/Characters/Hero.cpp
        src/Game/Characters/HeroStates.h
        src/Game/Characters/HeroDirections.cpp
        src/Game/Characters/HeroDirections.h
        src/Game/Characters/HeroStateMachine.cpp
        src/Game/Characters/HeroStateMachine.h
        src/Game/Characters/Components/HeroAnimComp.cpp
        src/Game/Characters/Components/InputComponent.cpp
        src/Game/Characters/Components/HeroMoveComp.cpp
        src/Game/Characters/Hand/Hand.cpp
        src/Game/Characters/Hand/Hand.h
)

set(GUNS_SOURCES
        src/Game/Guns/GunStates.h
        src/Game/Guns/Base/GunBase.cpp
        src/Game/Guns/Base/GunBase.h
        src/Game/Guns/RogueSpecial/RogueSpecial.cpp
        src/Game/Guns/RogueSpecial/RogueSpecial.h
        src/Game/Guns/VFX/MuzzleFlash.cpp
        src/Game/Guns/VFX/MuzzleFlash.h
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
        src/Game/Enemy/EnemyBase.cpp
        src/Game/Enemy/EnemyStates.h
        src/Game/Enemy/BulletMan/BulletManStates.h
        src/Game/Enemy/BulletMan/BulletManDirections.cpp
        src/Game/Enemy/BulletMan/BulletManDirections.h
        src/Game/Enemy/EnemyBase.h
        src/Game/Enemy/BulletMan/BulletMan.cpp
        src/Game/Enemy/BulletMan/BulletMan.h
        src/Game/Enemy/BulletMan/Components/BulletManAnimComp.cpp
        src/Game/Enemy/BulletMan/Components/BulletManAnimComp.h
        src/Game/Enemy/Components/EnemyMoveCompBase.cpp
        src/Game/Enemy/Components/EnemyMoveCompBase.h
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
