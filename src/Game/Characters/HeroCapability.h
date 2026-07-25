#pragma once
#include <cstdint>
#include "../Managers/Enum/FlagOperators.h"

namespace ETG
{
    //What the hero is allowed to do right now.
    //
    //NOTE: This replaces HeroStateFlags. That enum had to spell out both sides of every rule
    //(PreventShooting = Dash|Die|Hit *and* CanShoot = Idle|Run) because a flat state enum has nowhere to record
    //"these states belong together". In the state tree a capability is granted once on a parent node, inherited by
    //every descendant, and a single child can revoke it. So only the positive side is needed here.
    enum class HeroCapability : std::uint32_t
    {
        None = 0,                    // 000000 = 0
        CanMove = 1 << 0,            // 000001 = 1
        CanShoot = 1 << 1,           // 000010 = 2
        CanSwitchGuns = 1 << 2,      // 000100 = 4
        CanUseActiveItems = 1 << 3,  // 001000 = 8
        CanFlipAnims = 1 << 4,       // 010000 = 16
        CanTakeDamage = 1 << 5,      // 100000 = 32

        All = CanMove | CanShoot | CanSwitchGuns | CanUseActiveItems | CanFlipAnims | CanTakeDamage // 111111 = 63
    };
}
