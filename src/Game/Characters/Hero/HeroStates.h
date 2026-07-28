#pragma once
#include <boost/describe.hpp>

//The hero's enums, and only the hero's. Two different kinds live here:
//
//  HeroStateEnum        what the hero is *doing*. Leaf nodes of HeroStateMachine carry one of these
//  HeroRun/Idle/Dash/…  which art variant plays inside that state. These are animation keys, nothing else
//
//NOTE: These used to sit in Managers/Enum/StateEnums.h next to every other character's enums, and that file was
//pulled in by DirectionUtils.h, which almost everything includes. Touching BulletMan's death animation rebuilt
//the hero. Adding a boss means adding a file next to the boss, not editing one the hero reads
namespace ETG
{
    enum class HeroStateEnum
    {
        Idle,
        Run,
        Dash,
        Die,
        Hit
    };

    BOOST_DESCRIBE_ENUM(HeroStateEnum, Idle, Run, Dash, Die, Hit)

    enum class HeroRunEnum
    {
        Run_Back, // 0 = 00
        Run_BackWard, // 1 = 01
        Run_Forward, // 2 = 10
        Run_Front // 3 = 11
    };

    BOOST_DESCRIBE_ENUM(HeroRunEnum, Run_Back, Run_BackWard, Run_Forward, Run_Front)

    enum class HeroIdleEnum
    {
        Idle_Back,
        Idle_BackWard,
        Idle_Front,
        Idle_Right
    };

    BOOST_DESCRIBE_ENUM(HeroIdleEnum, Idle_Back, Idle_BackWard, Idle_Front, Idle_Right)

    enum class HeroDashEnum
    {
        Dash_Back,
        Dash_BackWard,
        Dash_Front,
        Dash_Left,
        Dash_Right,
        Unknown
    };

    BOOST_DESCRIBE_ENUM(HeroDashEnum, Dash_Back, Dash_BackWard, Dash_Front, Dash_Left, Dash_Right, Unknown)

    //I wish to have hit animations for all 8 directions
    enum class HeroHit
    {
        JustHit
    };

    BOOST_DESCRIBE_ENUM(HeroHit, JustHit)

    enum class HeroDeath
    {
        Dead
    };

    BOOST_DESCRIBE_ENUM(HeroDeath, Dead)
}
