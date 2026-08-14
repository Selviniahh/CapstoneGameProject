#include "HeroDirections.h"
#include "../../../Utils/Math.h"

namespace ETG
{
    Direction HeroDirections::LastDashDirection{};

    //NOTE: Both of these are mirror symmetric now: whatever Right gets, Left gets flipped, and the same for the two
    //diagonal pairs. They used to pair Right with DownLeft and Left with the back-diagonal sprite, so pointing the
    //mouse straight left drew the hero's back while pointing it straight right drew his side.
    //Down and Up are their own mirror image, so they simply pick the nearest sprite: the front one for Down, and the
    //dedicated back one for Up
    HeroIdleEnum HeroDirections::GetIdleEnum(const Direction currDir)
    {
        switch (currDir)
        {
        case Direction::Right:
        case Direction::Left:
            return HeroIdleEnum::Idle_Right;

        case Direction::DownRight:
        case Direction::Down:
        case Direction::DownLeft:
            return HeroIdleEnum::Idle_Front;

        case Direction::UpRight:
        case Direction::UpLeft:
            return HeroIdleEnum::Idle_BackWard;

        case Direction::Up:
            return HeroIdleEnum::Idle_Back;
        }

        return HeroIdleEnum::Idle_Back; // Default case
    }

    HeroRunEnum HeroDirections::GetRunEnum(const Direction currDir)
    {
        switch (currDir)
        {
        case Direction::Right:
        case Direction::Left:
            return HeroRunEnum::Run_Forward;

        case Direction::DownRight:
        case Direction::Down:
        case Direction::DownLeft:
            return HeroRunEnum::Run_Front;

        case Direction::UpRight:
        case Direction::UpLeft:
            return HeroRunEnum::Run_BackWard;

        case Direction::Up:
            return HeroRunEnum::Run_Back;
        }

        return HeroRunEnum::Run_Forward; // Default case
    }

    //Do not put any breakpoint at this function otherwise Key presses that captured in above GetDashDirectionEnum won't be captured during debugging.
    HeroDashEnum HeroDirections::GetDashEnum()
    {
        if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::D) && ETG::Keyboard::isKeyPressed(ETG::Keyboard::W))
        {
            LastDashDirection = Direction::UpRight;
            return HeroDashEnum::Dash_BackWard;
        }

        if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::A) && ETG::Keyboard::isKeyPressed(ETG::Keyboard::W))
        {
            LastDashDirection = Direction::UpLeft;
            return HeroDashEnum::Dash_BackWard;
        }
        if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::A) && ETG::Keyboard::isKeyPressed(ETG::Keyboard::S))
        {
            LastDashDirection = Direction::DownLeft;

            //Down-diagonals reuse the horizontal dash sheet, so the left one has to name the left enum. It named
            //Dash_Right before - a copy of the D+S branch below - and stayed invisible only because Dash_Left and
            //Dash_Right load the same sheet (HeroAnimComp.cpp:53-54) and the mirror comes off LastDashDirection
            return HeroDashEnum::Dash_Left;
        }
        if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::D) && ETG::Keyboard::isKeyPressed(ETG::Keyboard::S))
        {
            LastDashDirection = Direction::DownRight;
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
            LastDashDirection = Direction::Up;
            return HeroDashEnum::Dash_Back;
        }
        if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::S))
        {
            LastDashDirection = Direction::Down;
            return HeroDashEnum::Dash_Front;
        }
        return HeroDashEnum::Unknown;
    }

    //NOTE: The keys and the compass finally agree. W used to record BackHandRight and S a Direction::Front_For_Dash
    //that existed for no other reason, because the old names meant "straight up" here and "up and to the right"
    //when the same value came from the mouse. One enum, two readings
    ETG::Vector2f HeroDirections::GetDashVector() //Normalized vectors will be 0.707113562
    {
        switch (LastDashDirection)
        {
        case Direction::Right: return {1.0f, 0.0f};
        case Direction::Left: return {-1.0f, 0.0f};
        case Direction::Up: return {0.0f, -1.0f};
        case Direction::Down: return {0.0f, 1.0f};
        case Direction::DownRight: return Math::Normalize(ETG::Vector2f{1.0f, 1.0f});
        case Direction::DownLeft: return Math::Normalize(ETG::Vector2f{-1.0f, 1.0f}); //-0.7071 + 0.7
        case Direction::UpRight: return Math::Normalize(ETG::Vector2f{1.0f, -1.0f}); //0.7 -0.7
        case Direction::UpLeft: return Math::Normalize(ETG::Vector2f{-1.0f, -1.0f}); //-0.7 -0.7
        }

        return {0.0f, 0.0f};
    }
}
