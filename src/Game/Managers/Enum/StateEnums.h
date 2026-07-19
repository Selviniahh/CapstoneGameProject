#pragma once
#include <boost/describe.hpp>

namespace ETG
{
    enum class HeroRunEnum
    {
        Run_Back,
        Run_BackWard,
        Run_Forward,
        Run_Front
    };
    BOOST_DESCRIBE_ENUM(HeroRunEnum, Run_Back, Run_BackWard, Run_Forward, Run_Front)

    enum class HeroIdleEnum
    {
        Idle_Back,
        Idle_BackWard,
        Idle_Front,
        Idle_Right
    };
    BOOST_DESCRIBE_ENUM(HeroIdleEnum,Idle_Back, Idle_BackWard, Idle_Front, Idle_Right)

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

    enum class HeroStateEnum
    {
        Idle,
        Run,
        Dash,
        Die,
        Hit
    };
    BOOST_DESCRIBE_ENUM(HeroStateEnum, Idle, Run, Dash, Die, Hit)

    enum class EnemyStateEnum
    {
        Idle,
        Run,
        Dash,
        Die,
        Shooting,
        Hit
    };
    BOOST_DESCRIBE_ENUM(EnemyStateEnum, Idle, Run, Dash, Die, Shooting, Hit)

    enum class BulletManRunEnum
    {
        Run_Left,
        Run_Left_Back,
        Run_Right,
        Run_Right_Back
    };
    BOOST_DESCRIBE_ENUM(BulletManRunEnum, Run_Left, Run_Left_Back, Run_Right, Run_Right_Back)

    enum class BulletManIdleEnum
    {
        Idle_Back,
        Idle_Right,
        Idle_Left
    };
    BOOST_DESCRIBE_ENUM(BulletManIdleEnum, Idle_Back, Idle_Left, Idle_Right)

    enum class BulletManShootingEnum
    {
        Shoot_Left,
        Shoot_Right,
    };
    BOOST_DESCRIBE_ENUM(BulletManShootingEnum, Shoot_Left, Shoot_Right)

    enum class BulletManHitEnum
    {
        Hit_Back_Left,
        Hit_Back_Right,
        Hit_Left,
        Hit_Right
    };
    
    BOOST_DESCRIBE_ENUM(BulletManHitEnum, Hit_Back_Left, Hit_Back_Right, Hit_Left, Hit_Right)

    enum class BulletManDeathEnum
    {
        Death_Back_South,
        Death_Front_North,
        Death_Left_Back,
        Death_Left_Front,
        Death_Left_Side,
        Death_Right_Back,
        Death_Right_Front,
        Death_Right_Side
    };

    BOOST_DESCRIBE_ENUM(BulletManDeathEnum, Death_Back_South, Death_Front_North, Death_Left_Back, Death_Left_Front, Death_Left_Side, Death_Right_Back, Death_Right_Front, Death_Right_Side)
    
    enum class GunStateEnum
    {
        Idle,
        Shoot,
        Reload
    };
    BOOST_DESCRIBE_ENUM(GunStateEnum, Idle, Shoot, Reload)

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
