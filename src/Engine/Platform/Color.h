#pragma once
#include <cstdint>

namespace ETG
{
    //RGBA8 color replacement for sf::Color
    struct Color
    {
        std::uint8_t r = 255;
        std::uint8_t g = 255;
        std::uint8_t b = 255;
        std::uint8_t a = 255;

        constexpr Color() = default;

        constexpr Color(std::uint8_t R, std::uint8_t G, std::uint8_t B, std::uint8_t A = 255) : r(R), g(G), b(B), a(A)
        {
        }

        constexpr bool operator==(const Color& rhs) const { return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a; }
        constexpr bool operator!=(const Color& rhs) const { return !(*this == rhs); }

        static const Color White;
        static const Color Black;
        static const Color Red;
        static const Color Green;
        static const Color Blue;
        static const Color Yellow;
        static const Color Magenta;
        static const Color Cyan;
        static const Color Transparent;
    };

    inline const Color Color::White{255, 255, 255};
    inline const Color Color::Black{0, 0, 0};
    inline const Color Color::Red{255, 0, 0};
    inline const Color Color::Green{0, 255, 0};
    inline const Color Color::Blue{0, 0, 255};
    inline const Color Color::Yellow{255, 255, 0};
    inline const Color Color::Magenta{255, 0, 255};
    inline const Color Color::Cyan{0, 255, 255};
    inline const Color Color::Transparent{0, 0, 0, 0};
}
