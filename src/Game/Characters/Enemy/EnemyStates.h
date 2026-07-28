#pragma once
#include <boost/describe.hpp>

namespace ETG
{
    //What an enemy is doing. Shared by every enemy type; the per-type animation key enums live next to their
    //enemy instead (see BulletMan/BulletManStates.h)
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
}
