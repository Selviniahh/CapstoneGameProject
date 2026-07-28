#pragma once
#include "HeroStates.h"
#include "../../../Engine/Core/Direction.h"
#include "../../../Engine/Platform/Platform.h"

namespace ETG
{
    //Maps the hero's facing (and the dash keys) onto the hero's animation key enums.
    //
    //NOTE: This half used to live in DirectionUtils, one class that knew every character in the game and included
    //Hero.h from its .cpp. What is left in DirectionUtils now is only what is genuinely shared: angle -> Direction
    //and the direction-range map. Per-character mappings belong to the character
    class HeroDirections
    {
    public:
        static HeroIdleEnum GetIdleEnum(Direction currDir);
        static HeroRunEnum GetRunEnum(Direction currDir);

        //Reads the movement keys directly, so the hero can dash in a direction he is not facing. Also records
        //LastDashDirection, which is what the dash animation and the dash impulse are both resolved from
        static HeroDashEnum GetDashEnum();
        static ETG::Vector2f GetDashVector();

        //While dashing the hero faces where he is dashing, not where the mouse is
        static const Direction& GetDashFacing() { return LastDashDirection; }

    private:
        static Direction LastDashDirection;
    };
}
