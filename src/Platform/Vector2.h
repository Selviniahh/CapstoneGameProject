#pragma once
#include <cmath>

namespace ETG
{
    //2D vector replacement for sf::Vector2<T>, used across the whole game code.
    template <typename T>
    struct Vector2
    {
        T x{};
        T y{};

        constexpr Vector2() = default;

        constexpr Vector2(T X, T Y) : x(X), y(Y)
        {
        }

        //Converting constructor (e.g. Vector2u -> Vector2f)
        template <typename U>
        constexpr explicit Vector2(const Vector2<U>& other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y))
        {
        }

        constexpr bool operator==(const Vector2& rhs) const { return x == rhs.x && y == rhs.y; }
        constexpr bool operator!=(const Vector2& rhs) const { return !(*this == rhs); }

        constexpr Vector2 operator-() const { return {-x, -y}; }

        constexpr Vector2 operator+(const Vector2& rhs) const { return {x + rhs.x, y + rhs.y}; }
        constexpr Vector2 operator-(const Vector2& rhs) const { return {x - rhs.x, y - rhs.y}; }

        constexpr Vector2 operator*(T scalar) const { return {x * scalar, y * scalar}; }
        constexpr Vector2 operator/(T scalar) const { return {x / scalar, y / scalar}; }

        Vector2& operator+=(const Vector2& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }

        Vector2& operator-=(const Vector2& rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }

        Vector2& operator*=(T scalar)
        {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        Vector2& operator/=(T scalar)
        {
            x /= scalar;
            y /= scalar;
            return *this;
        }
    };

    template <typename T>
    constexpr Vector2<T> operator*(T scalar, const Vector2<T>& v)
    {
        return {v.x * scalar, v.y * scalar};
    }

    using Vector2f = Vector2<float>;
    using Vector2i = Vector2<int>;
    using Vector2u = Vector2<unsigned>;
}
