#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "src/Game/Managers/GameManager.h"

//The main entry point. The file has been kept as simple as possible. 
//The whole initialization has been handled in GameManager
int main(int argc, char* argv[])
{
    ETG::GameManager GM{};

    while (GM.IsRunning())
    {
        //Track every Frame
        GM.ProcessEvents();

        //The window might have been closed while processing events
        if (!GM.IsRunning())
            break;

        //While unfocused the game is paused: no simulation, no drawing. Only events are processed
        //so we can catch the focus-gained event and resume.
        if (!GM.WindowHasFocus())
        {
            SDL_Delay(10);
            continue;
        }

        GM.Update();
        GM.Draw();
    }

    
    return 0;
}
