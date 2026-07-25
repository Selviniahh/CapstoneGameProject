#include "BulletManDirections.h"

namespace ETG
{
    BulletManIdleEnum BulletManDirections::GetIdleEnum(const Direction currDir)
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

    BulletManRunEnum BulletManDirections::GetRunEnum(const Direction currDir)
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

    BulletManShootingEnum BulletManDirections::GetShootingEnum(const Direction currDir)
    {
        if (currDir == Direction::BackHandRight) return BulletManShootingEnum::Shoot_Right;
        if (currDir == Direction::BackDiagonalRight) return BulletManShootingEnum::Shoot_Right;
        if (currDir == Direction::Right) return BulletManShootingEnum::Shoot_Right;
        if (currDir == Direction::FrontHandRight) return BulletManShootingEnum::Shoot_Right;
        return BulletManShootingEnum::Shoot_Left; // else  return left
    }

    BulletManHitEnum BulletManDirections::GetHitEnum(const Direction currDir)
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

    BulletManDeathEnum BulletManDirections::GetDeathEnum(const Direction currDir)
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
}
