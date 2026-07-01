#pragma once
#include <algorithm>
#include "Vector2.h"

namespace ETG
{
    //Axis aligned rectangle replacement for sf::Rect<T> (left/top/width/height layout)
    template <typename T>
    struct Rect
    {
        T left{};
        T top{};
        T width{};
        T height{};

        constexpr Rect() = default;

        constexpr Rect(T Left, T Top, T Width, T Height) : left(Left), top(Top), width(Width), height(Height)
        {
        }

        constexpr Rect(const Vector2<T>& position, const Vector2<T>& size) : left(position.x), top(position.y), width(size.x), height(size.y)
        {
        }

        [[nodiscard]] constexpr Vector2<T> getPosition() const { return {left, top}; }
        [[nodiscard]] constexpr Vector2<T> getSize() const { return {width, height}; }

        constexpr bool operator==(const Rect& rhs) const
        {
            return left == rhs.left && top == rhs.top && width == rhs.width && height == rhs.height;
        }

        [[nodiscard]] bool contains(const Vector2<T>& point) const
        {
            const T minX = std::min(left, static_cast<T>(left + width));
            const T maxX = std::max(left, static_cast<T>(left + width));
            const T minY = std::min(top, static_cast<T>(top + height));
            const T maxY = std::max(top, static_cast<T>(top + height));
            return point.x >= minX && point.x < maxX && point.y >= minY && point.y < maxY;
        }

        [[nodiscard]] bool intersects(const Rect& other) const
        {
            Rect dummy;
            return intersects(other, dummy);
        }

        bool intersects(const Rect& other, Rect& overlap) const
        {
            const T interLeft = std::max(left, other.left);
            const T interTop = std::max(top, other.top);
            const T interRight = std::min(static_cast<T>(left + width), static_cast<T>(other.left + other.width));
            const T interBottom = std::min(static_cast<T>(top + height), static_cast<T>(other.top + other.height));

            if (interLeft < interRight && interTop < interBottom)
            {
                overlap = Rect(interLeft, interTop, interRight - interLeft, interBottom - interTop);
                return true;
            }

            overlap = Rect();
            return false;
        }
    };

    using IntRect = Rect<int>;
    using FloatRect = Rect<float>;
}
