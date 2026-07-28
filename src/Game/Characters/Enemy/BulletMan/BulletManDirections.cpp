#include "BulletManDirections.h"

//NOTE: Every mapping here is mirror symmetric: whatever the right half gets, the left half gets its mirror.
//It could not be before, because the Direction values themselves were not laid out symmetrically
namespace ETG
{
    //Three sprites for eight arcs, so the diagonals share with the cardinal they lean on
    BulletManIdleEnum BulletManDirections::GetIdleEnum(const Direction currDir)
    {
        switch (currDir)
        {
        case Direction::Up:
        case Direction::UpRight:
        case Direction::UpLeft:
            return BulletManIdleEnum::Idle_Back;

        case Direction::Right:
        case Direction::DownRight:
            return BulletManIdleEnum::Idle_Right;

        case Direction::Left:
        case Direction::DownLeft:
            return BulletManIdleEnum::Idle_Left;

        case Direction::Down:
            return BulletManIdleEnum::Idle_Right;
        }

        return BulletManIdleEnum::Idle_Back; // Default case
    }

    BulletManRunEnum BulletManDirections::GetRunEnum(const Direction currDir)
    {
        switch (currDir)
        {
        case Direction::Up:
        case Direction::UpRight:
            return BulletManRunEnum::Run_Right_Back;

        case Direction::UpLeft:
            return BulletManRunEnum::Run_Left_Back;

        case Direction::Right:
        case Direction::DownRight:
        case Direction::Down:
            return BulletManRunEnum::Run_Right;

        case Direction::Left:
        case Direction::DownLeft:
            return BulletManRunEnum::Run_Left;
        }

        return BulletManRunEnum::Run_Left; // Default case
    }

    BulletManShootingEnum BulletManDirections::GetShootingEnum(const Direction currDir)
    {
        return IsFacingRight(currDir) ? BulletManShootingEnum::Shoot_Right : BulletManShootingEnum::Shoot_Left;
    }

    BulletManHitEnum BulletManDirections::GetHitEnum(const Direction currDir)
    {
        switch (currDir)
        {
        case Direction::Up:
        case Direction::UpRight:
            return BulletManHitEnum::Hit_Back_Right;

        case Direction::UpLeft:
            return BulletManHitEnum::Hit_Back_Left;

        case Direction::Right:
        case Direction::DownRight:
        case Direction::Down:
            return BulletManHitEnum::Hit_Right;

        case Direction::Left:
        case Direction::DownLeft:
            return BulletManHitEnum::Hit_Left;
        }

        return BulletManHitEnum::Hit_Left; // else  return left
    }

    BulletManDeathEnum BulletManDirections::GetDeathEnum(const Direction currDir)
    {
        switch (currDir)
        {
        case Direction::Right:
            return BulletManDeathEnum::Death_Right_Side;

        case Direction::DownRight:
            return BulletManDeathEnum::Death_Right_Front;

        case Direction::Down:
            return BulletManDeathEnum::Death_Front_North;

        case Direction::DownLeft:
            return BulletManDeathEnum::Death_Left_Front;

        case Direction::Left:
            return BulletManDeathEnum::Death_Left_Side;

        case Direction::UpLeft:
            return BulletManDeathEnum::Death_Left_Back;

        case Direction::Up:
            return BulletManDeathEnum::Death_Back_South;

        case Direction::UpRight:
            return BulletManDeathEnum::Death_Right_Back;
        }

        // Added default case for safety
        return BulletManDeathEnum::Death_Back_South;
    }
}
