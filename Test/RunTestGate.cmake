# Runs one test executable as a build gate for the game, in script mode (cmake -P).
#
# It exists because a plain COMMAND in a custom target can only ever do one thing with a non-zero exit code: fail
# the build. Whether a red test should stop the build is a per-project decision (ETG_UNIT_TESTS_FAIL_BUILD /
# ETG_INTERACTIVE_TESTS_FAIL_BUILD), and this script is what turns that decision into either a FATAL_ERROR or a
# warning - portably, without shell tricks that differ between Windows and Unix.
#
# Called as:
#   cmake -DETG_GATE_LABEL=<name> -DETG_GATE_COMMAND=<exe> [-DETG_GATE_ARGS=<;-separated args>]
#         [-DETG_GATE_VERDICT_FILE=<path>] -DETG_GATE_FAIL_BUILD=<ON|OFF> -P RunTestGate.cmake
#
# ETG_GATE_VERDICT_FILE is optional and only the interactive host writes one. It holds the verdict the session
# reached, written the moment it was known. It is consulted ONLY when the process exited non-zero, to tell a
# genuine test failure apart from a process that reported success and then died on its way out - the engine's
# shutdown is not the tests' code, and a crash in it must not be reported as a failed gameplay check.

if(NOT ETG_GATE_COMMAND)
    message(FATAL_ERROR "RunTestGate.cmake: ETG_GATE_COMMAND is required")
endif()

# A verdict left over from the previous run would be read as this run's result
if(ETG_GATE_VERDICT_FILE AND EXISTS "${ETG_GATE_VERDICT_FILE}")
    file(REMOVE "${ETG_GATE_VERDICT_FILE}")
endif()

message(STATUS "[${ETG_GATE_LABEL}] running ${ETG_GATE_COMMAND} ${ETG_GATE_ARGS}")

execute_process(
        COMMAND ${ETG_GATE_COMMAND} ${ETG_GATE_ARGS}
        RESULT_VARIABLE gate_result
)

if(gate_result EQUAL 0)
    message(STATUS "[${ETG_GATE_LABEL}] passed")
    return()
endif()

# Non-zero. If the session wrote a verdict before the process died, that verdict is the truth about the tests
if(ETG_GATE_VERDICT_FILE AND EXISTS "${ETG_GATE_VERDICT_FILE}")
    file(READ "${ETG_GATE_VERDICT_FILE}" gate_verdict)
    string(STRIP "${gate_verdict}" gate_verdict)

    if(gate_verdict STREQUAL "OK")
        message(WARNING
                "[${ETG_GATE_LABEL}] the tests passed, but the process exited badly (${gate_result}) - that is the "
                "engine's shutdown, not a test result. Continuing.")
        return()
    endif()

    message(STATUS "[${ETG_GATE_LABEL}] verdict: ${gate_verdict}")
endif()

if(ETG_GATE_FAIL_BUILD)
    message(FATAL_ERROR
            "[${ETG_GATE_LABEL}] FAILED (exit code ${gate_result}). The game was not built.\n"
            "Turn this gate off with -DETG_RUN_UNIT_TESTS_BEFORE_GAME=OFF / "
            "-DETG_RUN_INTERACTIVE_TESTS_BEFORE_GAME=OFF, or let it fail without stopping the build with "
            "-DETG_UNIT_TESTS_FAIL_BUILD=OFF / -DETG_INTERACTIVE_TESTS_FAIL_BUILD=OFF.")
else()
    message(WARNING "[${ETG_GATE_LABEL}] failed (exit code ${gate_result}), continuing anyway.")
endif()
