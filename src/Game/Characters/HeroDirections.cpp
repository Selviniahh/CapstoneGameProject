#include "HeroDirections.h"
#include "../../Utils/Math.h"

namespace ETG
{
    Direction HeroDirections::LastDashDirection{};

    HeroIdleEnum HeroDirections::GetIdleEnum(const Direction currDir)
    {
        if (currDir == Direction::BackHandRight || currDir == Direction::BackHandLeft) return HeroIdleEnum::Idle_Back;
        if (currDir == Direction::BackDiagonalRight || currDir == Direction::BackDiagonalLeft) return HeroIdleEnum::Idle_BackWard;
        if (currDir == Direction::Right || currDir == Direction::Left) return HeroIdleEnum::Idle_Right;
        if (currDir == Direction::FrontHandRight || currDir == Direction::FrontHandLeft) return HeroIdleEnum::Idle_Front;
        return HeroIdleEnum::Idle_Back; // Default case
    }

    HeroRunEnum HeroDirections::GetRunEnum(const Direction currDir)
    {
        if (currDir == Direction::BackHandRight || currDir == Direction::BackHandLeft) return HeroRunEnum::Run_Back;
        if (currDir == Direction::BackDiagonalRight || currDir == Direction::BackDiagonalLeft) return HeroRunEnum::Run_BackWard;
        if (currDir == Direction::Right || currDir == Direction::Left) return HeroRunEnum::Run_Forward;
        if (currDir == Direction::FrontHandRight || currDir == Direction::FrontHandLeft) return HeroRunEnum::Run_Front;
        return HeroRunEnum::Run_Forward; // Default case
    }

    //Do not put any breakpoint at this function otherwise Key presses that captured in above GetDashDirectionEnum won't be captured during debugging.
    HeroDashEnum HeroDirections::GetDashEnum()
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

    ETG::Vector2f HeroDirections::GetDashVector() //Normalized vectors will be 0.707113562
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
}
