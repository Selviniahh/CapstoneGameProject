#pragma once
#include "FlagOperators.h"

//NOTE: The hero half of this file is gone. Its states now live in a hierarchical state machine
//(src/Engine/Core/StateMachine) and its permissions in HeroCapability.h, where every rule is stated once instead
//of twice. The enemy still uses the flat flag approach; docs/StateMachine.md describes how to move it over.
namespace ETG
{
    enum class EnemyStateFlag
    {
        StateNone = 0,
        StateIdle = 1 << 0,
        StateRun = 1 << 1,
        StateShooting = 1 << 2,
        StateHit = 1 << 3,
        StateDie = 1 << 4,

        PreventMovement = StateDie | StateHit,
        PreventShooting = StateDie | StateHit,
        PreventAnimFlip = StateDie,

        CanMove = StateIdle | StateRun | StateShooting,
        CanShoot = StateIdle | StateRun | StateShooting,
        CanFlipAnims = StateIdle | StateRun | StateShooting | StateHit
    };
}
