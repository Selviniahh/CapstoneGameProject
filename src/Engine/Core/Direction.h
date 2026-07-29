#pragma once
#include <boost/describe.hpp>

namespace ETG
{
    //8-way facing, in screen space: 0 degrees is right and the angle grows clockwise because +y points down.
    //Each value owns a 45 degree arc centred on itself, so Right is [337.5, 22.5), DownRight is [22.5, 67.5) and
    //so on. DirectionUtils is the only thing that turns an angle into one of these.
    //
    //NOTE: These names used to be Right / FrontHandRight / FrontHandLeft / Left / BackDiagonalLeft / BackHandLeft /
    //BackHandRight / BackDiagonalRight - four Left/Right pairs. Eight arcs centred on the compass points only give
    //you three mirror pairs though, because straight down and straight up are their own mirror image, so four pairs
    //could not be laid out symmetrically: the left half of the compass ended up rotated 45 degrees against the right
    //half. Pointing the mouse straight left drew the back-diagonal sprite while pointing it straight right drew the
    //side one. The names carried the sprite variant *and* the flip; they now carry neither, and each animation
    //component states out loud which sprite and which flip a direction means.
    enum class Direction
    {
        Right,     //   0
        DownRight, //  45
        Down,      //  90
        DownLeft,  // 135
        Left,      // 180
        UpLeft,    // 225
        Up,        // 270
        UpRight    // 315
    };

    BOOST_DESCRIBE_ENUM(Direction, Right, DownRight, Down, DownLeft, Left, UpLeft, Up, UpRight)

    //Whether a sprite facing this way should be drawn unflipped. The art is drawn facing right.
    //
    //NOTE: This used to be BaseAnimComp::IsFacingRight, which decided by asking whether the enum's *name* contained
    //the substring "Right". That is why the names had to spell out a side, and therefore why they could not also be
    //a plain compass. Down and Up are their own mirror image, so their flip is a free choice - the two picked here
    //are the ones the game already had, they do not mean anything.
    [[nodiscard]] constexpr bool IsFacingRight(const Direction direction)
    {
        switch (direction)
        {
        case Direction::Right:
        case Direction::DownRight:
        case Direction::UpRight:
        case Direction::Up:
            return true;

        case Direction::Left:
        case Direction::DownLeft:
        case Direction::UpLeft:
        case Direction::Down:
            return false;
        }

        return true;
    }

    //Whether a character facing this way is drawn from behind. The three upward arcs are the ones with a back
    //sprite (Up gets the dedicated one, UpLeft and UpRight the three-quarter one), and they are exactly the
    //facings where something the character holds in front of its body belongs behind it in the draw order.
    [[nodiscard]] constexpr bool IsFacingBack(const Direction direction)
    {
        return direction == Direction::Up || direction == Direction::UpLeft || direction == Direction::UpRight;
    }
}
