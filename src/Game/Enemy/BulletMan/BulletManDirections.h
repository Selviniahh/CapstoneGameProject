#pragma once
#include "BulletManStates.h"
#include "../../../Engine/Core/Direction.h"

namespace ETG
{
    //Maps a BulletMan's facing onto its animation key enums. The next enemy gets its own file like this one
    //instead of another five methods on a shared utility class
    class BulletManDirections
    {
    public:
        static BulletManIdleEnum GetIdleEnum(Direction currDir);
        static BulletManRunEnum GetRunEnum(Direction currDir);
        static BulletManShootingEnum GetShootingEnum(Direction currDir);
        static BulletManHitEnum GetHitEnum(Direction currDir);
        static BulletManDeathEnum GetDeathEnum(Direction currDir);
    };
}
