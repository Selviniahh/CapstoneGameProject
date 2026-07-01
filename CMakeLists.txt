cmake_minimum_required(VERSION 3.28)

project(ETG LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

# ------------------------------------------------------------------------------
# Dependencies (no package manager required)
#  - SDL3: fetched from the official release tarball and built from source
#  - Dear ImGui (+ SDL3 backends): vendored under external/imgui
#  - stb (image/vorbis/truetype decoding): vendored under external/stb
#  - Boost (header-only: type_index, describe, mpl): fetched release tarball
# ------------------------------------------------------------------------------
include(FetchContent)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        SDL3
        URL https://www.libsdl.org/release/SDL3-3.4.12.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(SDL3)

# Boost: the release tarball ships all headers pre-assembled under boost/.
# The three libraries used here (type_index, describe, mpl) are header-only.
FetchContent_Declare(
        boost_headers
        URL https://archives.boost.io/release/1.91.0/source/boost_1_91_0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(boost_headers)

add_library(boost_header_only INTERFACE)
target_include_directories(boost_header_only SYSTEM INTERFACE ${boost_headers_SOURCE_DIR})
add_library(Boost::type_index ALIAS boost_header_only)
add_library(Boost::describe ALIAS boost_header_only)
add_library(Boost::mpl ALIAS boost_header_only)

# Dear ImGui with the SDL3 + SDL_Renderer3 backends (vendored)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
add_library(imgui STATIC
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(imgui PUBLIC SDL3::SDL3)

# stb single-header libraries (implementations live in src/Platform/StbImpl.cpp)
add_library(stb INTERFACE)
target_include_directories(stb SYSTEM INTERFACE ${CMAKE_SOURCE_DIR}/external/stb)

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
        src/Platform/StbImpl.cpp
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

# Create a master STATIC library for all SDL-dependent code
add_library(etgcore STATIC
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

# Common definitions for the static library
target_compile_definitions(etgcore PRIVATE
        RESOURCE_PATH="../../Resources"
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

# Link everything to the static library
target_link_libraries(etgcore PUBLIC
        SDL3::SDL3
        imgui
        stb
        Boost::type_index
        Boost::describe
        Boost::mpl
)

add_library(utils STATIC ${UTILS_SOURCES})
target_compile_definitions(utils PRIVATE RESOURCE_PATH="${CMAKE_SOURCE_DIR}/Resources")
target_include_directories(utils PUBLIC src)
target_link_libraries(utils PUBLIC Boost::type_index Boost::describe Boost::mpl SDL3::SDL3 imgui etg_compile_flags)

add_library(modifiers INTERFACE)
target_sources(modifiers INTERFACE ${MODIFIERS_SOURCES})
target_compile_definitions(modifiers INTERFACE RESOURCE_PATH="${CMAKE_SOURCE_DIR}/Resources")
target_include_directories(modifiers INTERFACE src)

add_executable(ETG main.cpp)

target_compile_definitions(ETG PRIVATE RESOURCE_PATH="${CMAKE_SOURCE_DIR}/Resources")
target_link_libraries(ETG PRIVATE
        etgcore  # Our consolidated static library
        utils
        modifiers
        SDL3::SDL3
        imgui
        Boost::type_index
        Boost::describe
        Boost::mpl
        Threads::Threads
)
