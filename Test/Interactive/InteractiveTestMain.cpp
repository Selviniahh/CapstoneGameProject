#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include "Framework/InteractiveTestRunner.h"
#include "Game/Managers/GameManager.h"

//=====================================================================================================================
//  ETGInteractiveTests - the interactive gameplay test host.
//
//  This is the real game engine, booted with an EMPTY world. Where the game spawns its level
//  (SpawnInitialLevel::Spawn), this spawns one object: the test runner. Everything else you see - the hero, the
//  enemies, the guns - was spawned by whichever test is loaded, and disappears when you switch tests.
//
//  That is the whole point of the separate executable: a new mechanic gets a test that builds exactly the world it
//  needs, and the game's own level is never touched to try something out.
//
//  Usage:
//      ETGInteractiveTests                    play through the tests, exits 0 unless a check FAILED
//      ETGInteractiveTests --strict           also exit non-zero if any test was left unplayed or pending
//      ETGInteractiveTests --verdict-file=P   write the verdict to P as well as returning it
//
//  The exit code is what makes this usable as a build gate: with
//  -DETG_RUN_INTERACTIVE_TESTS_BEFORE_GAME=ON the game's build launches this first, and a failed check stops the
//  build before the game ever starts. See Test/README.md.
//
//  --verdict-file exists because an exit code cannot tell "the tests failed" apart from "the process died on its
//  way out". The engine's shutdown is not this test host's code and has been seen to crash after the session is
//  over (a driver teardown on a software GL stack, say); with the verdict on disk the gate can tell the two apart
//  instead of blaming the tests for it.
//=====================================================================================================================

namespace
{
    //One turn of the loop. Identical to the game's own (main.cpp) - this host differs in what is in the world,
    //not in how the world is ticked
    bool Tick(ETG::GameManager& game)
    {
        game.ProcessEvents();

        if (!game.IsRunning())
            return false;

        if (!game.WindowHasFocus())
        {
            SDL_Delay(10);
            return true;
        }

        game.Update();
        game.Draw();
        return true;
    }

    //Written the moment the verdict is known, before anything is torn down
    void WriteVerdictFile(const std::string& path, const char* verdict)
    {
        if (path.empty()) return;

        if (std::ofstream file{path}; file)
            file << verdict << "\n";
        else
            std::cerr << "Could not write the verdict file: " << path << "\n";
    }
}

int main(int argc, char* argv[])
{
    bool strict = false;
    std::string verdictFile;

    for (int i = 1; i < argc; ++i)
    {
        const std::string argument = argv[i];

        if (argument == "--strict") strict = true;
        else if (argument.starts_with("--verdict-file=")) verdictFile = argument.substr(std::strlen("--verdict-file="));
    }

    //THE hook. Set before the GameManager is constructed, because the constructor is what spawns the world.
    //The runner is the only thing spawned here; the tests spawn the rest
    ETG::GameManager::LevelSpawnOverride = [](ETG::GameManager& game)
    {
        game.SpawnGameObject<ETG::Testing::InteractiveTestRunner>(game);
    };

    ETG::GameManager game{};

    while (Tick(game))
    {
    }

    //<---------- The verdict ---------->
    const ETG::Testing::InteractiveTestRunner* runner = ETG::Testing::InteractiveTestRunner::Get();
    if (!runner)
    {
        std::cout << "Interactive test host exited before a runner existed.\n";
        WriteVerdictFile(verdictFile, "OK");
        return 0;
    }

    runner->PrintSummary();

    if (runner->AnyCheckFailed())
    {
        std::cout << "Result: FAILED - a gameplay check failed, or a test could not be built.\n";
        WriteVerdictFile(verdictFile, "FAILED");
        return 1;
    }

    if (strict && !runner->AllTestsFullyResolved())
    {
        std::cout << "Result: INCOMPLETE - --strict requires every test to be played to a verdict.\n";
        WriteVerdictFile(verdictFile, "INCOMPLETE");
        return 2;
    }

    std::cout << "Result: OK\n";
    WriteVerdictFile(verdictFile, "OK");
    return 0;
}
