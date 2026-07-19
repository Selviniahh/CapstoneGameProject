#pragma once
#include "../Engine/Managers/Time.h"
#include "../Engine/Platform/Platform.h"
#include <complex>
#include <numbers>
#include "../Engine/Managers/RenderContext.h"
#include <iostream>
#include <random>

class Math
{
    Math() = delete;

public:
    template <typename T>
    static T GenRandomNumber(const T& min, const T& max)
    {
        static std::mt19937 gen(std::random_device{}());

        //Integer specialization
        if constexpr (std::is_integral_v<T>)
        {
            std::uniform_int_distribution<T> dis(min, max);
            return dis(gen);
        }
        //floating point specialization
        else if constexpr (std::is_floating_point_v<T>)
        {
            std::uniform_real_distribution<T> dis(min, max);
            return dis(gen);
        }
    }

    //length: Compute the magnitude of the vector using the formula sqrt(x^2 + y^2)
    // Division: Divide the vector components by the magnitude to scale it to a unit vector (length 1).
    template <typename T>
    static inline ETG::Vector2<T> Normalize(const ETG::Vector2<T>& Vector)
    {
        const float length = VectorLength(Vector);
        if (length == 0) throw std::runtime_error("length is 0. Vector is: " + std::to_string(Vector.x) + " " + std::to_string(Vector.y));

        return Vector / length;
    }

    template <typename T>
    static inline float RadiansToDegrees(T radians)
    {
        return radians * (180.0f / std::numbers::pi);
    }

    static float AngleToRadian(const float angle)
    {
        return (angle * std::numbers::pi) / 180.f;
    }

    static ETG::Vector2f RadianToDirection(const float rad)
    {
        return {std::cos(rad), std::sin(rad)};
    }

    template <typename T>
    static inline T VectorSizeSquared(const ETG::Vector2<T>& Vector)
    {
        return Vector.x * Vector.x + Vector.y * Vector.y;
    }

    static float Length(const ETG::Vector2f& vector)
    {
        return std::sqrt(vector.x * vector.x + vector.y * vector.y);
    }

    template <typename T>
    static float VectorLength(ETG::Vector2<T> vector)
    {
        return std::sqrt(VectorSizeSquared(vector));
    }

    template <typename T>
    static bool IsInRange(const T& value, const T& min, const T& max)
    {
        return value >= min && value <= max;
    }

    //------------------------Trigonometry--------------------------------------------------------

    //Used for non internally incremented timer
    //look at ReloadSlider
    template <typename T>
    static T SinWaveLerp(T a, T b, T interval, float& timer)
    {
        //Update the timer (0 to  and back)
        timer += ETG::Time::FrameTick / interval;
        if (timer > 1.0f) timer = 0.0f;

        //Multiplying by π transforms this range into [0, π]
        //When timer = 0.5: sin(π/2) = 1 → fully at position b
        //When timer = 1.0: sin(π) = 0 → back to midpoint
        float sineValue = std::sin(timer * std::numbers::pi);

        //apply lerp
        return static_cast<T>(std::lerp(a, b, sineValue));
    }

    //Used for internally incremented timer
    //Normally Timer needs 1 second to reach from 0 -> 1
    //If interval is 10. reaching 0 -> 1 will take 10 seconds
    //Look at FrameLeftProgressBar
    template <typename T>
    static T IntervalLerp(const T& a, const T& b, const T& interval, float timer)
    {
        timer /= interval;
        if (timer > 1.0f) timer = 0.0f;

        //apply lerp
        return static_cast<T>(std::lerp(a, b, timer));
    }

    // Returns a bell curve value that starts at 0, peaks at progress=0.5, and returns to 0
    // progress should be between 0 and 1
    static float BellCurve(const float progress)
    {
        return std::sin(progress * std::numbers::pi);
    }

    // Applies a bell curve force to create smooth dash movement
    // progress: Current progress of the dash (0 to 1)
    // direction: Normalized direction vector
    // amount: Maximum force at peak
    // returns: The velocity to apply for this frame
    static ETG::Vector2f ApplyBellCurveForce(const float progress, const ETG::Vector2f& direction, const float amount, const float deltaTime)
    {
        // Calculate force using bell curve (peaks at 0.5 progress)
        const float force = BellCurve(progress) * amount;

        // Return velocity for this frame
        return direction * force * deltaTime;
    }

    static float AngleBetween(const ETG::Vector2f& from, const ETG::Vector2f& to)
    {
        float deltaY = to.y - from.y;
        float deltaX = to.x - from.x;
        float angleRadians = std::atan2(deltaY, deltaX);
        return RadiansToDegrees(angleRadians);
    }

    //-----------------------------Transformations-----------------------------------------------------

    [[nodiscard]] static ETG::Vector2f RotateVector(const float rotation, const ETG::Vector2f scale, const ETG::Vector2f& offset)
    {
        const float angleRad = rotation * (std::numbers::pi / 180.f);
        ETG::Vector2f scaledOffset(offset.x * scale.x, offset.y * scale.y);

        return {
            scaledOffset.x * std::cos(angleRad) - scaledOffset.y * std::sin(angleRad),
            scaledOffset.x * std::sin(angleRad) + scaledOffset.y * std::cos(angleRad)
        };
    }

    struct FourCorner
    {
        ETG::Vector2f TopLeft{};
        ETG::Vector2f TopRight{};
        ETG::Vector2f BottomLeft{};
        ETG::Vector2f BottomRight{};
    };


    [[nodiscard]] static FourCorner CalculateFourCorner(ETG::Vector2f& pos, const ETG::Vector2f& size, const ETG::Vector2f& origin, const ETG::Vector2f& scale = {1.f, 1.f})
    {
        FourCorner Corners;

        //Calculate the scaled size firstly
        const ETG::Vector2f scaledSize = {size.x * scale.x, size.y * scale.y};

        //Calculate corners using the scaled size
        Corners.TopLeft = {pos.x, pos.y}; // Top-left
        Corners.TopRight = {pos.x + scaledSize.x, pos.y}; // Top-right
        Corners.BottomLeft = {pos.x, pos.y + scaledSize.y}; // Bottom-left
        Corners.BottomRight = {pos.x + scaledSize.x, pos.y + scaledSize.y}; // Bottom-right

        //Origin also always affected by scale
        ETG::Vector2f scaledOrigin = {origin.x * scale.x, origin.y * scale.y};
        Corners.TopLeft -= scaledOrigin;
        Corners.TopRight -= scaledOrigin;
        Corners.BottomLeft -= scaledOrigin;
        Corners.BottomRight -= scaledOrigin;
        return Corners;
    }

    template <typename T>
    static T CalculatePercentageOfValue(const T& value, const float& percentage)
    {
        return value * (percentage / 100);
    }
};
