#pragma once
#include <type_traits>

//Bitwise helpers shared by the flag-style enums in the game.
//NOTE: These used to be unconstrained templates, which made them overload candidates for *every* type in
//namespace ETG via ADL. The requires-clause keeps them to enums, where they were always meant to live.
namespace ETG
{
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
