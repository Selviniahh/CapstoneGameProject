#pragma once
#include <boost/describe.hpp>

namespace ETG
{
    //NOTE: Still a flat enum. Unlike the hero, a gun has no state tree yet, and ReloadSlider - a UI object -
    //assigns Gun->CurrentGunState directly. That is the same layering violation the hero just got rid of
    enum class GunStateEnum
    {
        Idle,
        Recoil,
        Shoot,
        Reload
    };

    BOOST_DESCRIBE_ENUM(GunStateEnum, Idle, Recoil, Shoot, Reload)
}
