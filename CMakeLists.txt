cmake_minimum_required(VERSION 3.28)
if (NOT DEFINED ENV{VCPKG_CMAKE})
        message(FATAL_ERROR "VCPKG_CMAKE environment variable is not set.")
endif ()

set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_CMAKE}" CACHE STRING "Vcpkg toolchain file")
project(ETG LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

# Make GNU path format to resolve any binary copy paste requirement for SFML
include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

# Specify only source files
add_executable(ETG main.cpp
        src/UI/UserInterface.cpp
        src/Managers/Globals.cpp
        src/Animation/Animation.cpp
        src/Managers/GameManager.cpp
        src/Characters/Hero.cpp
        src/Characters/Components/HeroAnimComp.cpp
        src/Characters/Components/InputComponent.cpp
        src/Characters/Components/HeroMoveComp.cpp
        src/Guns/Base/GunBase.cpp
        src/Guns/Base/GunBase.h
        src/Core/GameObjectBase.h
        src/Projectile/ProjectileBase.h
        src/Projectile/ProjectileBase.cpp
        src/Guns/RogueSpecial/RogueSpecial.cpp
        src/Guns/RogueSpecial/RogueSpecial.h
        src/Managers/GameState.h
        src/Utils/DirectionUtils.cpp
        src/Utils/DirectionUtils.h
        src/Enemy/EnemyBase.cpp
        src/Enemy/EnemyBase.h
        src/Core/Components/BaseHealthComp.cpp
        src/Core/Components/BaseHealthComp.h
        src/Core/Components/BaseMoveComp.cpp
        src/Core/Components/BaseMoveComp.h
        src/Utils/Math.h
        src/Managers/SpriteBatch.cpp
        src/Core/Components/BaseAnimComp.h
        src/Engine/Engine.cpp
        src/Engine/Engine.h
        src/Core/GameObjectBase.cpp
        src/Managers/InputManager.cpp
        src/Engine/UI/EngineUI.cpp
        src/Engine/UI/EngineUI.h
        src/Core/Factory.h
        src/Core/Scene/Scene.h
        src/Utils/TextureUtils.h
        src/Core/ComponentBase.h
        src/Utils/Interface/IAnimationComponent.h
        src/Utils/StrManipulateUtil.h
        src/Managers/TypeRegistry.h
        src/Engine/UI/UIUtils.h
        src/Engine/Reflection.h
        src/Core/GameClass.h
        src/Engine/UI/UIUtils.cpp
        src/Characters/Hand/Hand.cpp
        src/Characters/Hand/Hand.h
        src/Enemy/BulletMan/BulletMan.cpp
        src/Enemy/BulletMan/BulletMan.h
        src/Enemy/BulletMan/Components/BulletManAnimComp.cpp
        src/Enemy/BulletMan/Components/BulletManAnimComp.h
        src/Guns/Magnum/Magnum.cpp
        src/Guns/Magnum/Magnum.h
        src/Items/ActiveItem.h
        src/Items/ActiveItem.cpp
        src/Items/PassiveItem.h
        src/Items/PassiveItem.cpp
)

# Easiest and fastest way is VCPKG 
find_package(SFML CONFIG REQUIRED system window graphics network audio)
find_package(benchmark CONFIG REQUIRED)
find_package(Threads REQUIRED)
find_package(ImGui-SFML CONFIG REQUIRED)
find_package(boost_type_index CONFIG REQUIRED)
find_package(boost_describe CONFIG REQUIRED)
find_package(boost_mpl REQUIRED CONFIG)


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" STREQUAL "MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif ()

target_compile_options(ETG PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:-ftemplate-backtrace-limit=0 -ferror-limit=0>
        $<$<CXX_COMPILER_ID:Clang>:-ftemplate-backtrace-limit=0 -ferror-limit=0>
)

#Because in different builds the final path of the build directory differs, detecting Resources path from Cmake is better option.
target_compile_definitions(ETG PRIVATE RESOURCE_PATH="${CMAKE_SOURCE_DIR}/Resources")

target_link_libraries(ETG PRIVATE
        sfml-audio
        sfml-graphics
        $<$<PLATFORM_ID:Windows>:sfml-main>  # Only link sfml-main on Windows
        sfml-network
        sfml-system
        sfml-window
        benchmark::benchmark
        ImGui-SFML::ImGui-SFML
        Boost::type_index
        Boost::describe
        Boost::mpl
)

install(TARGETS ETG
        RUNTIME DESTINATION bin  # For Windows executables
)
