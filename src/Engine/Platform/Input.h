#pragma once
#include "Vector2.h"

namespace ETG
{
    class RenderWindow;

    //Keyboard state polling, replacement for sf::Keyboard
    namespace Keyboard
    {
        enum Key
        {
            A,
            D,
            E,
            Q,
            R,
            S,
            W,
            Space,
            Up,
            Down,
            Left,
            Right
        };

        bool isKeyPressed(Key key);
    }

    //Mouse state polling, replacement for sf::Mouse
    namespace Mouse
    {
        enum Button
        {
            Left,
            Right,
            Middle
        };

        bool isButtonPressed(Button button);

        //Mouse position on the desktop (global coordinates)
        Vector2i getPosition();

        //Mouse position relative to the given window
        Vector2i getPosition(const RenderWindow& window);
    }
}
