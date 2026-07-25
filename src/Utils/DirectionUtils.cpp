#include "DirectionUtils.h"
#include <stdexcept>

#include "Math.h"

void ETG::DirectionUtils::PopulateDirectionRanges(DirectionMap mapToFill)
{
    mapToFill[{0, 22}] = Direction::Right;
    mapToFill[{22, 67}] = Direction::FrontHandRight;
    mapToFill[{67, 112}] = Direction::FrontHandLeft;
    mapToFill[{112, 157}] = Direction::Left;
    mapToFill[{157, 202}] = Direction::BackDiagonalLeft;
    mapToFill[{202, 247}] = Direction::BackHandLeft;
    mapToFill[{247, 292}] = Direction::BackHandRight;
    mapToFill[{292, 337}] = Direction::BackDiagonalRight;
    mapToFill[{337, 360}] = Direction::Right;
}

ETG::Direction ETG::DirectionUtils::GetDirectionToTarget(const ETG::Vector2f& targetPosition, const ETG::Vector2f& selfPosition)
{
    const ETG::Vector2f dirVector = Math::Normalize(targetPosition - selfPosition);

    // Calculate angle in degrees (0-360)
    float angle = atan2(dirVector.y, dirVector.x) * 180.0f / std::numbers::pi;
    if (angle < 0) angle += 360.0f;

    // Map angle to direction (each direction covers 45 degrees) Right is 0 degrees, and we go counter-clockwise
    if (angle >= 337.5f || angle < 22.5f)
        return Direction::Right;
    else if (angle >= 22.5f && angle < 67.5f)
        return Direction::FrontHandRight;
    else if (angle >= 67.5f && angle < 112.5f)
        return Direction::FrontHandLeft;
    else if (angle >= 112.5f && angle < 157.5f)
        return Direction::Left;
    else if (angle >= 157.5f && angle < 202.5f)
        return Direction::BackDiagonalLeft;
    else if (angle >= 202.5f && angle < 247.5f)
        return Direction::BackHandLeft;
    else if (angle >= 247.5f && angle < 292.5f)
        return Direction::BackHandRight;
    else
        return Direction::BackDiagonalRight;
}

ETG::Direction ETG::DirectionUtils::GetDirectionFromAngle(const std::unordered_map<std::pair<int, int>, Direction, PairHash>& DirectionMap, const float angle)
{
    //The first std::pair is key and element. Second std::pair is the key's type itself
    for (const auto& [fst, snd] : DirectionMap)
    {
        //Check if angle within any defined range
        if (angle >= fst.first && angle <= fst.second)
        {
            return snd;
        }
    }
    throw std::out_of_range("Mouse angle is out of defined ranges. Angle is: " + std::to_string(angle));
}
