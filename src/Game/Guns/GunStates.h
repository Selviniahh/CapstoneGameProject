#pragma once
#include <boost/describe.hpp>

namespace ETG
{
    // NOTE: Hâlâ flat enum kullanılıyor. Hero'nun aksine silahın henüz state tree'si yoktur ve bir UI object
    // olan ReloadSlider, Gun->CurrentGunState değerini doğrudan atar. Bu, hero'da yeni kaldırılanla aynı
    // layering violation'dır.
    enum class GunStateEnum
    {
        Idle,
        Recoil,
        Shoot,
        Reload
    };

    BOOST_DESCRIBE_ENUM(GunStateEnum, Idle, Recoil, Shoot, Reload)
}
