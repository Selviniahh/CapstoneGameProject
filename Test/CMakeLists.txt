# ======================================================================================================================
#  Test/ - the two kinds of tests, and the switches that decide when they run.
#
#  Two executables are produced, and they are independent of each other and of the game:
#
#    ETGUnitTests         GoogleTest, console only, no window. Pure logic (angles, stats, maths)
#    ETGInteractiveTests  the real engine booted with an EMPTY world, one test at a time building its own
#
#  Four switches, set at configure time (-D...=ON/OFF) or from the IDE's CMake profile:
#
#    ETG_BUILD_UNIT_TESTS                    build ETGUnitTests at all                        (default ON)
#    ETG_BUILD_INTERACTIVE_TESTS             build ETGInteractiveTests at all                 (default ON)
#    ETG_RUN_UNIT_TESTS_BEFORE_GAME          run them as part of building the game            (default OFF)
#    ETG_RUN_INTERACTIVE_TESTS_BEFORE_GAME   launch them as part of building the game         (default OFF)
#
#  The last two are the "before runtime" gates: with a gate on, building/running ETG runs that test suite first and
#  a failure stops the build, so the game never starts on a red test. With both off, ETG builds and runs exactly as
#  it always did. See Test/README.md for the full matrix.
# ======================================================================================================================

# ---- Sources ---------------------------------------------------------------------------------------------------
# Add a new unit test file here
set(ETG_UNIT_TEST_SOURCES
        Unit/DirectionTests.cpp
        Unit/StatModifierTests.cpp
        Unit/MathTests.cpp
)

# The interactive framework. Rarely changes - the tests below are what grows
set(ETG_INTERACTIVE_FRAMEWORK_SOURCES
        Interactive/Framework/InteractiveTest.h
        Interactive/Framework/InteractiveTest.cpp
        Interactive/Framework/InteractiveTestRegistry.h
        Interactive/Framework/InteractiveTestRegistry.cpp
        Interactive/Framework/InteractiveTestRunner.h
        Interactive/Framework/InteractiveTestRunner.cpp
        Interactive/Framework/TestEnvironment.h
        Interactive/Framework/TestEnvironment.cpp
        Interactive/Framework/TestWatchers.h
        Interactive/Framework/TestWatchers.cpp
)

# ADD A NEW INTERACTIVE TEST HERE. That plus the ETG_INTERACTIVE_TEST(...) line at the bottom of the file is all
# it takes for it to appear in the panel
set(ETG_INTERACTIVE_TEST_SOURCES
        Interactive/Tests/HeroDirectionTest.cpp
        Interactive/Tests/GunShotSpeedTest.cpp
        Interactive/Tests/EnemyEngagementTest.cpp
)

# ======================================================================================================================
#  Unit tests (GoogleTest)
# ======================================================================================================================
if(ETG_BUILD_UNIT_TESTS)
    include(${CMAKE_SOURCE_DIR}/cmake/depBuild/googletest.cmake)

    if(NOT ETG_GTEST_AVAILABLE)
        # A missing submodule disables the unit tests instead of breaking the whole configure: somebody who only
        # wants to build the game should not be stopped by a dependency only the tests need
        message(WARNING "deps/googletest is missing, unit tests are disabled. "
                "Run: git submodule update --init deps/googletest")
        set(ETG_BUILD_UNIT_TESTS OFF)
    endif()
endif()

if(ETG_BUILD_UNIT_TESTS)
    add_executable(ETGUnitTests ${ETG_UNIT_TEST_SOURCES})

    # etgcore/utils carry the code under test; gtest_main provides main()
    target_link_libraries(ETGUnitTests PRIVATE
            etgcore
            utils
            modifiers
            GTest::gtest
            GTest::gtest_main
    )

    # Registered with CTest as one test, so `ctest` in the build directory runs the suite. Individual cases are
    # still selectable the normal way: ETGUnitTests --gtest_filter=StatModifier.*
    add_test(NAME ETGUnitTests COMMAND ETGUnitTests)

    if(WIN32)
        # Windows only looks next to the executable for its DLLs, and the project's libraries are shared
        add_custom_command(TARGET ETGUnitTests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        $<TARGET_RUNTIME_DLLS:ETGUnitTests>
                        $<TARGET_FILE_DIR:ETGUnitTests>
                COMMAND_EXPAND_LISTS
                COMMENT "Copying runtime DLLs next to ETGUnitTests"
        )
    endif()
endif()

# ======================================================================================================================
#  Interactive gameplay tests
# ======================================================================================================================
if(ETG_BUILD_INTERACTIVE_TESTS)
    # NOTE: the test sources are compiled straight into the executable rather than into a library on purpose. Each
    # test registers itself from a namespace-scope object, and a linker is free to drop an object file from a
    # static library when nothing references it - which would silently drop the test from the panel's list
    add_executable(ETGInteractiveTests
            Interactive/InteractiveTestMain.cpp
            ${ETG_INTERACTIVE_FRAMEWORK_SOURCES}
            ${ETG_INTERACTIVE_TEST_SOURCES}
    )

    # The framework's own headers are included relative to each other ("../Framework/..."), and everything from the
    # game is included from the source root ("Game/Characters/Hero/Hero.h") - which etgcore already exposes
    target_include_directories(ETGInteractiveTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

    target_link_libraries(ETGInteractiveTests PRIVATE
            etgcore
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

    # The host loads the same assets the game does, through AssetManager (which resolves relative to the
    # executable). Both binaries land in the same bin/ directory, so this is the same copy the game does - repeated
    # here so the interactive tests can be built and run without building the game at all
    add_custom_command(TARGET ETGInteractiveTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                    ${CMAKE_SOURCE_DIR}/Resources
                    $<TARGET_FILE_DIR:ETGInteractiveTests>/Resources
            COMMENT "Copying Resources next to ETGInteractiveTests"
    )

    if(WIN32)
        add_custom_command(TARGET ETGInteractiveTests POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        $<TARGET_RUNTIME_DLLS:ETGInteractiveTests>
                        $<TARGET_FILE_DIR:ETGInteractiveTests>
                COMMAND_EXPAND_LISTS
                COMMENT "Copying runtime DLLs next to ETGInteractiveTests"
        )
    endif()
endif()

# ======================================================================================================================
#  The "before runtime" gates
#
#  A gate is a custom target the game target depends on. It has no outputs, so it runs on EVERY build of ETG, and
#  the game is only built after it has passed. Turning a gate off removes the dependency entirely - nothing about
#  the game's own build changes.
# ======================================================================================================================
set(ETG_GATE_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/RunTestGate.cmake)

if(ETG_RUN_UNIT_TESTS_BEFORE_GAME)
    if(NOT TARGET ETGUnitTests)
        message(FATAL_ERROR "ETG_RUN_UNIT_TESTS_BEFORE_GAME needs ETG_BUILD_UNIT_TESTS=ON (and deps/googletest checked out)")
    endif()

    add_custom_target(etg_gate_unit_tests
            COMMAND ${CMAKE_COMMAND}
                    -DETG_GATE_LABEL=unit-tests
                    -DETG_GATE_COMMAND=$<TARGET_FILE:ETGUnitTests>
                    -DETG_GATE_FAIL_BUILD=${ETG_UNIT_TESTS_FAIL_BUILD}
                    -P ${ETG_GATE_SCRIPT}
            COMMENT "Gate: unit tests before the game"
            USES_TERMINAL
            VERBATIM
    )

    # The gate runs the executable, so it has to be built first; ETG in turn waits on the gate
    add_dependencies(etg_gate_unit_tests ETGUnitTests)
    add_dependencies(ETG etg_gate_unit_tests)
endif()

if(ETG_RUN_INTERACTIVE_TESTS_BEFORE_GAME)
    if(NOT TARGET ETGInteractiveTests)
        message(FATAL_ERROR "ETG_RUN_INTERACTIVE_TESTS_BEFORE_GAME needs ETG_BUILD_INTERACTIVE_TESTS=ON")
    endif()

    # Where the session records what it concluded. Read by the gate script only when the process exits non-zero,
    # so a crash in the engine's shutdown is not mistaken for a failed gameplay check
    set(ETG_INTERACTIVE_VERDICT_FILE ${CMAKE_CURRENT_BINARY_DIR}/interactive-tests-verdict.txt)

    # --strict additionally demands that every test was played to a verdict, not merely opened
    set(ETG_INTERACTIVE_GATE_ARGS "--verdict-file=${ETG_INTERACTIVE_VERDICT_FILE}")
    if(ETG_INTERACTIVE_TESTS_STRICT)
        list(APPEND ETG_INTERACTIVE_GATE_ARGS "--strict")
    endif()

    add_custom_target(etg_gate_interactive_tests
            COMMAND ${CMAKE_COMMAND}
                    -DETG_GATE_LABEL=interactive-tests
                    -DETG_GATE_COMMAND=$<TARGET_FILE:ETGInteractiveTests>
                    "-DETG_GATE_ARGS=${ETG_INTERACTIVE_GATE_ARGS}"
                    -DETG_GATE_VERDICT_FILE=${ETG_INTERACTIVE_VERDICT_FILE}
                    -DETG_GATE_FAIL_BUILD=${ETG_INTERACTIVE_TESTS_FAIL_BUILD}
                    -P ${ETG_GATE_SCRIPT}
            COMMENT "Gate: interactive gameplay tests before the game (close the window to continue)"
            USES_TERMINAL
            VERBATIM
    )

    add_dependencies(etg_gate_interactive_tests ETGInteractiveTests)
    add_dependencies(ETG etg_gate_interactive_tests)
endif()
