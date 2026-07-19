#include "DirectionUtils.h"
#include <stdexcept>

#include "Math.h"
#include "../Game/Managers/Enum/StateEnums.h"
#include "../Game/Characters/Hero.h"

ETG::Direction ETG::DirectionUtils::LastDashDirection{};

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

ETG::Direction ETG::DirectionUtils::GetDirectionToHero(const Hero* Hero, const ETG::Vector2f SelfPosition)
{
    const ETG::Vector2f dirVector = Math::Normalize(Hero->GetPosition() - SelfPosition);

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

ETG::Direction ETG::DirectionUtils::GetHeroDirectionFromAngle(const std::unordered_map<std::pair<int, int>, Direction, PairHash>& DirectionMap, const float angle)
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

ETG::HeroIdleEnum ETG::DirectionUtils::GetHeroIdleDirectionEnum(Direction currDir)
{
    if (currDir == Direction::BackHandRight || currDir == Direction::BackHandLeft) return HeroIdleEnum::Idle_Back;
    if (currDir == Direction::BackDiagonalRight || currDir == Direction::BackDiagonalLeft) return HeroIdleEnum::Idle_BackWard;
    if (currDir == Direction::Right || currDir == Direction::Left) return HeroIdleEnum::Idle_Right;
    if (currDir == Direction::FrontHandRight || currDir == Direction::FrontHandLeft) return HeroIdleEnum::Idle_Front;
    return HeroIdleEnum::Idle_Back; // Default case
}

ETG::HeroRunEnum ETG::DirectionUtils::GetHeroRunEnum(Direction currDir)
{
    if (currDir == Direction::BackHandRight || currDir == Direction::BackHandLeft) return HeroRunEnum::Run_Back;
    if (currDir == Direction::BackDiagonalRight || currDir == Direction::BackDiagonalLeft) return HeroRunEnum::Run_BackWard;
    if (currDir == Direction::Right || currDir == Direction::Left) return HeroRunEnum::Run_Forward;
    if (currDir == Direction::FrontHandRight || currDir == Direction::FrontHandLeft) return HeroRunEnum::Run_Front;
    return HeroRunEnum::Run_Forward; // Default case
}

ETG::BulletManShootingEnum ETG::DirectionUtils::GetBulletManShootingEnum(Direction currDir)
{
    if (currDir == Direction::BackHandRight) return BulletManShootingEnum::Shoot_Right;
    if (currDir == Direction::BackDiagonalRight) return BulletManShootingEnum::Shoot_Right;
    if (currDir == Direction::Right) return BulletManShootingEnum::Shoot_Right;
    if (currDir == Direction::FrontHandRight) return BulletManShootingEnum::Shoot_Right;
    return BulletManShootingEnum::Shoot_Left; // else  return left
}

ETG::BulletManHitEnum ETG::DirectionUtils::GetBulletManHitEnum(Direction currDir)
{
    if (currDir == Direction::BackHandRight) return BulletManHitEnum::Hit_Back_Right;
    if (currDir == Direction::BackDiagonalRight) return BulletManHitEnum::Hit_Back_Right;
    if (currDir == Direction::Right) return BulletManHitEnum::Hit_Right;
    if (currDir == Direction::FrontHandRight) return BulletManHitEnum::Hit_Right;
    if (currDir == Direction::BackHandLeft) return BulletManHitEnum::Hit_Back_Left;
    if (currDir == Direction::BackDiagonalLeft) return BulletManHitEnum::Hit_Back_Left;
    if (currDir == Direction::Left) return BulletManHitEnum::Hit_Left;
    if (currDir == Direction::FrontHandLeft) return BulletManHitEnum::Hit_Left;

    return BulletManHitEnum::Hit_Left; // else  return left
}

ETG::BulletManDeathEnum ETG::DirectionUtils::GetBulletManDeathEnum(Direction currDir)
{
    switch (currDir)
    {
    case Direction::Right:
        return BulletManDeathEnum::Death_Right_Side;

    case Direction::FrontHandRight:
        return BulletManDeathEnum::Death_Right_Front;

    case Direction::FrontHandLeft:
        return BulletManDeathEnum::Death_Left_Front;

    case Direction::Left:
        return BulletManDeathEnum::Death_Left_Side;

    case Direction::BackDiagonalLeft:
        return BulletManDeathEnum::Death_Left_Back;

    case Direction::BackHandLeft:
        return BulletManDeathEnum::Death_Left_Back;

    case Direction::BackHandRight:
        return BulletManDeathEnum::Death_Right_Back;

    case Direction::BackDiagonalRight:
        return BulletManDeathEnum::Death_Right_Back;

    case Direction::Front_For_Dash:
        return BulletManDeathEnum::Death_Front_North;

    default:
        // Added default case for safety
        return BulletManDeathEnum::Death_Back_South;
    }
}

//Do not put any breakpoint at this function otherwise Key presses that captured in above GetDashDirectionEnum won't be captured during debugging. 
ETG::HeroDashEnum ETG::DirectionUtils::GetDashDirectionEnum()
{
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::D) && ETG::Keyboard::isKeyPressed(ETG::Keyboard::W))
    {
        LastDashDirection = Direction::BackDiagonalRight;
        return HeroDashEnum::Dash_BackWard;
    }

    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::A) && ETG::Keyboard::isKeyPressed(ETG::Keyboard::W))
    {
        LastDashDirection = Direction::BackDiagonalLeft;
        return HeroDashEnum::Dash_BackWard;
    }
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::A) && ETG::Keyboard::isKeyPressed(ETG::Keyboard::S))
    {
        LastDashDirection = Direction::FrontHandLeft;

        return HeroDashEnum::Dash_Right;
    }
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::D) && ETG::Keyboard::isKeyPressed(ETG::Keyboard::S))
    {
        LastDashDirection = Direction::FrontHandRight;
        return HeroDashEnum::Dash_Right;
    }
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::A))
    {
        LastDashDirection = Direction::Left;
        return HeroDashEnum::Dash_Left;
    }
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::D))
    {
        LastDashDirection = Direction::Right;
        return HeroDashEnum::Dash_Right;
    }
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::W))
    {
        LastDashDirection = Direction::BackHandRight;
        return HeroDashEnum::Dash_Back;
    }
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::S))
    {
        LastDashDirection = Direction::Front_For_Dash;
        return HeroDashEnum::Dash_Front;
    }
    return HeroDashEnum::Unknown;
}

ETG::Vector2f ETG::DirectionUtils::GetDashDirectionVector() //Normalized vectors will be 0.707113562
{
    switch (LastDashDirection)
    {
    case Direction::Left: return {-1.0f, 0.0f};
    case Direction::Right: return {1.0f, 0.0f};
    case Direction::BackHandRight: return {0.0f, -1.0f};
    case Direction::BackHandLeft: return {0.0f, -1.0f};
    case Direction::FrontHandRight: return Math::Normalize(ETG::Vector2f{1.0f, 1.0f});
    case Direction::FrontHandLeft: return Math::Normalize(ETG::Vector2f{-1.0f, 1.0f}); //-0.7071 + 0.7
    case Direction::BackDiagonalRight: return Math::Normalize(ETG::Vector2f{1.0f, -1.0f});; //0.7 -0.7
    case Direction::BackDiagonalLeft: return Math::Normalize(ETG::Vector2f{-1.0, -1.0f}); //-0.7 -0.7
    case Direction::Front_For_Dash: return {0, 1};
    default:
        return {0.0f, 0.0f};
    }
}


ETG::BulletManIdleEnum ETG::DirectionUtils::GetBulletManIdleEnum(Direction currDir)
{
    if (currDir == Direction::BackHandRight || currDir == Direction::BackHandLeft ||
        currDir == Direction::BackDiagonalRight)
        return BulletManIdleEnum::Idle_Back;

    if (currDir == Direction::Right || currDir == Direction::FrontHandRight)
        return BulletManIdleEnum::Idle_Right;

    if (currDir == Direction::Left || currDir == Direction::FrontHandLeft || currDir == Direction::BackDiagonalLeft)
        return BulletManIdleEnum::Idle_Left;

    return BulletManIdleEnum::Idle_Back; // Default case
}

ETG::BulletManRunEnum ETG::DirectionUtils::GetBulletManRunEnum(Direction currDir)
{
    if (currDir == Direction::BackHandRight || currDir == Direction::BackDiagonalRight)
        return BulletManRunEnum::Run_Right_Back;

    if (currDir == Direction::BackHandLeft || currDir == Direction::BackDiagonalLeft)
        return BulletManRunEnum::Run_Left_Back;

    if (currDir == Direction::Right || currDir == Direction::FrontHandRight)
        return BulletManRunEnum::Run_Right;

    if (currDir == Direction::Left || currDir == Direction::FrontHandLeft)
        return BulletManRunEnum::Run_Left;

    return BulletManRunEnum::Run_Left; // Default case
}
