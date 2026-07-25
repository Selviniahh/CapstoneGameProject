#include "DirectionUtils.h"
#include <stdexcept>

#include "Math.h"

namespace
{
    //Every arc is 45 degrees wide and centred on its compass point, so its edges sit half a sector either side.
    //Right is centred on 0, which is the one arc that has to be written as two entries
    constexpr float SectorHalf = 22.5f;

    ETG::DirectionUtils::DirectionRanges MakeDefaultRanges()
    {
        using ETG::Direction;

        return {
            {{360.f - SectorHalf, 360.f}, Direction::Right},
            {{0.f, SectorHalf}, Direction::Right},
            {{45.f - SectorHalf, 45.f + SectorHalf}, Direction::DownRight},
            {{90.f - SectorHalf, 90.f + SectorHalf}, Direction::Down},
            {{135.f - SectorHalf, 135.f + SectorHalf}, Direction::DownLeft},
            {{180.f - SectorHalf, 180.f + SectorHalf}, Direction::Left},
            {{225.f - SectorHalf, 225.f + SectorHalf}, Direction::UpLeft},
            {{270.f - SectorHalf, 270.f + SectorHalf}, Direction::Up},
            {{315.f - SectorHalf, 315.f + SectorHalf}, Direction::UpRight},
        };
    }
}

ETG::DirectionUtils::DirectionRanges& ETG::DirectionUtils::GetRanges()
{
    static DirectionRanges ranges = MakeDefaultRanges();
    return ranges;
}

void ETG::DirectionUtils::ResetRangesToDefault()
{
    GetRanges() = MakeDefaultRanges();
}

ETG::Direction ETG::DirectionUtils::GetDirectionFromAngle(const float angle)
{
    for (const auto& [range, direction] : GetRanges())
        if (angle >= range.first && angle < range.second)
            return direction;

    throw std::out_of_range("Angle is outside every direction range. Angle is: " + std::to_string(angle));
}

float ETG::DirectionUtils::GetAngleToTarget(const ETG::Vector2f& targetPosition, const ETG::Vector2f& origin)
{
    const ETG::Vector2f difference = targetPosition - origin;

    float angle = Math::RadiansToDegrees(std::atan2(difference.y, difference.x));
    if (angle < 0.f) angle += 360.f;

    return angle;
}

ETG::Direction ETG::DirectionUtils::GetDirectionToTarget(const ETG::Vector2f& targetPosition, const ETG::Vector2f& selfPosition)
{
    return GetDirectionFromAngle(GetAngleToTarget(targetPosition, selfPosition));
}
