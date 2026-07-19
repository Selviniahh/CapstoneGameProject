#pragma once
#include <boost/describe.hpp>

namespace ETG
{
    //8-way facing direction used by the engine's animation components (BaseAnimComp flip logic).
    //Lives on the engine side; game state enums (StateEnums.h) re-export it for convenience.
    enum class Direction
    {
        Right,
        FrontHandRight,
        FrontHandLeft,
        Left,
        BackDiagonalLeft,
        BackHandLeft,
        BackHandRight,
        BackDiagonalRight,
        Front_For_Dash //This will only set when Dashing with S key other than this, this value will never be set again
    };
    BOOST_DESCRIBE_ENUM(Direction, Right, FrontHandRight, FrontHandLeft, Left, BackDiagonalLeft, BackHandLeft, BackHandRight, BackDiagonalRight, Front_For_Dash)
}
