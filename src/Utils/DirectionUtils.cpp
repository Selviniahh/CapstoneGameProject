#include "DirectionUtils.h"
#include <stdexcept>

#include "Math.h"
#include "../Managers/StateEnums.h"
#include "../Characters/Hero.h"


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

ETG::Direction ETG::DirectionUtils::GetDirectionToHero(const Hero* Hero, sf::Vector2f SelfPosition)
{
    const sf::Vector2f dirVector = Math::Normalize(Hero->GetPosition() - SelfPosition);

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

// Add these implementations to your DirectionUtils.cpp file

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