#pragma once
#include <boost/describe.hpp>

namespace ETG
{
    //8-way facing direction used by the engine's animation components (BaseAnimComp flip logic).
    //Lives on the engine side. Game code that needs it includes this header directly; it used to arrive by accident
    //through StateEnums.h, which every character shared.
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
