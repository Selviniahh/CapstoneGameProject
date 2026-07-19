#include "Input.h"
#include <SDL3/SDL.h>
#include "RenderWindow.h"

namespace ETG
{
    namespace Keyboard
    {
        static SDL_Scancode ToScancode(const Key key)
        {
            switch (key)
            {
            case A: return SDL_SCANCODE_A;
            case D: return SDL_SCANCODE_D;
            case E: return SDL_SCANCODE_E;
            case Q: return SDL_SCANCODE_Q;
            case R: return SDL_SCANCODE_R;
            case S: return SDL_SCANCODE_S;
            case W: return SDL_SCANCODE_W;
            case Space: return SDL_SCANCODE_SPACE;
            case Up: return SDL_SCANCODE_UP;
            case Down: return SDL_SCANCODE_DOWN;
            case Left: return SDL_SCANCODE_LEFT;
            case Right: return SDL_SCANCODE_RIGHT;
            }
            return SDL_SCANCODE_UNKNOWN;
        }

        bool isKeyPressed(const Key key)
        {
            const bool* state = SDL_GetKeyboardState(nullptr);
            return state && state[ToScancode(key)];
        }
    }

    namespace Mouse
    {
        bool isButtonPressed(const Button button)
        {
            const SDL_MouseButtonFlags flags = SDL_GetMouseState(nullptr, nullptr);
            switch (button)
            {
            case Left: return flags & SDL_BUTTON_LMASK;
            case Right: return flags & SDL_BUTTON_RMASK;
            case Middle: return flags & SDL_BUTTON_MMASK;
            }
            return false;
        }

        Vector2i getPosition()
        {
            float x = 0.f, y = 0.f;
            SDL_GetGlobalMouseState(&x, &y);
            return {static_cast<int>(x), static_cast<int>(y)};
        }

        Vector2i getPosition(const RenderWindow& window)
        {
            //SDL_GetMouseState is relative to the window with keyboard/mouse focus,
            //which is the game window in practice (single window application).
            (void)window;
            float x = 0.f, y = 0.f;
            SDL_GetMouseState(&x, &y);
            return {static_cast<int>(x), static_cast<int>(y)};
        }
    }
}
