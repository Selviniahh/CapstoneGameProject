#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "src/Game/Managers/GameManager.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

//The main entry point. The file has been kept as simple as possible.
//The whole initialization has been handled in GameManager
namespace
{
    //One turn of the game loop. Returns false once the window has been closed.
    bool Tick(ETG::GameManager& GM)
    {
        //Track every Frame
        GM.ProcessEvents();

        //The window might have been closed while processing events
        if (!GM.IsRunning())
            return false;

        //While unfocused the game is paused: no simulation, no drawing. Only events are processed
        //so we can catch the focus-gained event and resume.
        if (!GM.WindowHasFocus())
        {
#ifndef __EMSCRIPTEN__
            //Sleeping is not an option inside a browser frame callback; there the frame is just skipped.
            SDL_Delay(10);
#endif
            return true;
        }

        GM.Update();
        GM.Draw();
        return true;
    }

#ifdef __EMSCRIPTEN__
    ETG::GameManager* WebGameManager = nullptr;

    void WebTick()
    {
        if (!Tick(*WebGameManager))
            emscripten_cancel_main_loop();
    }
#endif
}

int main(int argc, char* argv[])
{
    ETG::GameManager GM{};

#ifdef __EMSCRIPTEN__
    //In the browser the frame clock belongs to the page: hand the loop to requestAnimationFrame
    //instead of blocking on it, otherwise nothing would ever be painted.
    WebGameManager = &GM;
    emscripten_set_main_loop(WebTick, 0, true);
#else
    while (Tick(GM))
    {
    }
#endif

    return 0;
}
