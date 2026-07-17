#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "src/Managers/GameManager.h"

int main(int argc, char* argv[])
{
    ETG::GameManager GM{};

    while (ETG::GameManager::IsRunning())
    {
        //Track every Frame
        GM.ProcessEvents();

        //The window might have been closed while processing events
        if (!ETG::GameManager::IsRunning()) break;

        //If the window unfocused, sleep the thread
        if (!GM.WindowHasFocus())
        {
            SDL_Delay(10);
        }

        GM.Update();
        GM.Draw();
    }

    return 0;
}
