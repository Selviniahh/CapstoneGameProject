#pragma once
#include <boost/describe.hpp>

//BulletMan's animation keys. Which art variant plays inside a given EnemyStateEnum; nothing here is a state.
//A new enemy gets its own file like this one, so deleting an enemy is deleting a folder
namespace ETG
{
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
}
