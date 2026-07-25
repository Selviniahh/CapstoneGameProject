#pragma once
#include <type_traits>

//Bitwise helpers shared by the flag-style enums in the game.
//NOTE: These used to be unconstrained templates, which made them overload candidates for *every* type in
//namespace ETG via ADL. The requires-clause keeps them to enums, where they were always meant to live.

/* enum class HeroRunEnum
// {
//     Run_Back,      // 0 = 00
//     Run_BackWard,  // 1 = 01
//     Run_Forward,   // 2 = 10
//     Run_Front      // 3 = 11
// };
//
// Dolayısıyla örneğin:
//
// auto result =
//     HeroRunEnum::Run_BackWard |
//     HeroRunEnum::Run_Forward;
//
// İşlem:
//
//   01  // Run_BackWard = 1
// | 10  // Run_Forward  = 2
// ----
//   11  // 3
//
// Sonuç:
//
// result == HeroRunEnum::Run_Front */

namespace ETG
{
     // enum a ──→ tam sayı ─┐
     //                      ├── bitwise OR ──→ tam sayı ──→ enum
     // enum b ──→ tam sayı ─┘
    template <typename T> requires std::is_enum_v<T>
    constexpr T operator|(const T a, const T b)
    {
        return static_cast<T>(static_cast<std::underlying_type_t<T>>(a) | static_cast<std::underlying_type_t<T>>(b));
    }

    template <typename T> requires std::is_enum_v<T>
    constexpr T operator&(const T a, const T b)
    {
        return static_cast<T>(static_cast<std::underlying_type_t<T>>(a) & static_cast<std::underlying_type_t<T>>(b));
    }

    template <typename T> requires std::is_enum_v<T>
    constexpr T operator~(const T a)
    {
        return static_cast<T>(~static_cast<std::underlying_type_t<T>>(a));
    }

    template <typename T> requires std::is_enum_v<T>
    constexpr bool HasAnyFlag(const T flags, const T check)
    {
        return static_cast<std::underlying_type_t<T>>(flags & check) != 0;
    }
}
    