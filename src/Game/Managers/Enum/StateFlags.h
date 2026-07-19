#pragma once

namespace ETG
{
    enum class HeroStateFlags
    {
        StateNone = 0,
        StateIdle = 1 << 0, //1
        StateRun = 1 << 1, //2
        StateDash = 1 << 2, //4
        StateDie = 1 << 3, //8
        StateHit = 1 << 4, //16

        PreventGunSwitching = StateDash | StateDie | StateHit,
        PreventMovement = StateDash | StateDie | StateHit,
        PreventShooting = StateDash | StateDie | StateHit,
        PreventAnimFlip = StateDie | StateHit,
        PreventActiveItemUsage = StateDie | StateHit | StateDash,
        PreventDamage = StateDie | StateHit | StateDash,

        CanGunSwitch = StateIdle | StateRun,
        CanMove = StateIdle | StateRun,
        CanShoot = StateIdle | StateRun,
        CanFlipAnims = StateDash | StateIdle | StateRun,
        CanUseActiveItems = StateIdle | StateRun,
        CanTakeDamage = StateIdle | StateRun | StateDash,
    };

    enum class EnemyStateFlag
    {
        StateNone = 0,
        StateIdle = 1 << 0,
        StateRun = 1 << 1,
        StateShooting = 1 << 2,
        StateHit = 1 << 3,
        StateDie = 1 << 4,
    
        PreventMovement =  StateDie | StateHit,
        PreventShooting = StateDie | StateHit,
        PreventAnimFlip = StateDie,
    
        CanMove = StateIdle | StateRun | StateShooting,
        CanShoot = StateIdle | StateRun | StateShooting,
        CanFlipAnims = StateIdle | StateRun | StateShooting | StateHit
    };

    //Helper functions for flag manipulation
    template<typename T>
    T operator|(const T a, const T b)
    {
        return static_cast<T>(static_cast<int>(a) | static_cast<int>(b));
    }

    template<typename T>
    T operator&(const T a, const T b)
    {
        return static_cast<T>(static_cast<int>(a) & static_cast<int>(b));
    }

    template<typename T>
    inline bool HasAnyFlag(const T flags, const T check)
    {
        return (flags & check) != T::StateNone;
    }

    inline bool HasAllFlags(const HeroStateFlags flags, const HeroStateFlags check)
    {
        return (flags & check) == check;
    }
}
