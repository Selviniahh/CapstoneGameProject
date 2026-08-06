#include <gtest/gtest.h>
#include "Engine/Core/Direction.h"
#include "Utils/DirectionUtils.h"

//=====================================================================================================================
//  UNIT TESTS for the 8-way facing maths.
//
//  This is the half of the direction system that CAN be asserted without a window: an angle goes in, a Direction
//  comes out, and the answer is either right or wrong. Whether the hero actually turns when the mouse moves is the
//  other half, and it is the interactive test's job (Test/Interactive/Tests/HeroDirectionTest.cpp).
//
//  That split is worth keeping in mind when adding tests: if a question can be answered by calling a function with
//  numbers, it belongs here, because here it is answered on every build without anybody playing anything.
//=====================================================================================================================

using namespace ETG;

namespace
{
    //Each arc is 45 degrees wide and centred on its compass point, so the centre is the least surprising sample
    TEST(DirectionFromAngle, CompassCentresResolveToTheirOwnDirection)
    {
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(0.f), Direction::Right);
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(45.f), Direction::DownRight);
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(90.f), Direction::Down);
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(135.f), Direction::DownLeft);
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(180.f), Direction::Left);
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(225.f), Direction::UpLeft);
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(270.f), Direction::Up);
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(315.f), Direction::UpRight);
    }

    //The arcs are half open, [min, max). An angle landing exactly on a boundary belongs to the arc ABOVE it, and
    //to only one arc - the bug this pins down is the old inclusive-both-ends table, where a boundary angle matched
    //two arcs and the winner came down to hash order
    TEST(DirectionFromAngle, BoundariesBelongToTheArcAbove)
    {
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(22.5f), Direction::DownRight);
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(67.5f), Direction::Down);
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(112.5f), Direction::DownLeft);
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(157.5f), Direction::Left);
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(202.5f), Direction::UpLeft);
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(247.5f), Direction::Up);
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(292.5f), Direction::UpRight);
    }

    //Right is the one direction written as two entries, because its arc straddles 0
    TEST(DirectionFromAngle, RightWrapsAroundZero)
    {
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(350.f), Direction::Right);
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(359.9f), Direction::Right);
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(10.f), Direction::Right);
    }

    //Every angle in [0, 360) has to resolve. The function throws when it cannot, which is what this asserts
    TEST(DirectionFromAngle, EveryAngleInRangeResolves)
    {
        for (int degrees = 0; degrees < 360; ++degrees)
            EXPECT_NO_THROW(DirectionUtils::GetDirectionFromAngle(static_cast<float>(degrees)));
    }

    //Screen space: +y points down, so an angle grows clockwise on screen
    TEST(AngleToTarget, MeasuredClockwiseFromRight)
    {
        constexpr ETG::Vector2f origin{0.f, 0.f};

        EXPECT_FLOAT_EQ(DirectionUtils::GetAngleToTarget({10.f, 0.f}, origin), 0.f);
        EXPECT_FLOAT_EQ(DirectionUtils::GetAngleToTarget({0.f, 10.f}, origin), 90.f);
        EXPECT_FLOAT_EQ(DirectionUtils::GetAngleToTarget({-10.f, 0.f}, origin), 180.f);
        EXPECT_FLOAT_EQ(DirectionUtils::GetAngleToTarget({0.f, -10.f}, origin), 270.f);
    }

    //What an enemy calls to face the hero. Same table as the hero's mouse angle goes through - that is the point
    TEST(DirectionToTarget, FacesTheTarget)
    {
        constexpr ETG::Vector2f self{100.f, 100.f};

        EXPECT_EQ(DirectionUtils::GetDirectionToTarget({200.f, 100.f}, self), Direction::Right);
        EXPECT_EQ(DirectionUtils::GetDirectionToTarget({100.f, 200.f}, self), Direction::Down);
        EXPECT_EQ(DirectionUtils::GetDirectionToTarget({0.f, 100.f}, self), Direction::Left);
        EXPECT_EQ(DirectionUtils::GetDirectionToTarget({100.f, 0.f}, self), Direction::Up);
        EXPECT_EQ(DirectionUtils::GetDirectionToTarget({200.f, 200.f}, self), Direction::DownRight);
    }

    //The sprite flip. Three mirror pairs plus Down and Up, which are their own mirror image and whose flip is
    //therefore a free choice - these are the values the game happens to use
    TEST(FacingHelpers, IsFacingRightMatchesTheArtwork)
    {
        EXPECT_TRUE(IsFacingRight(Direction::Right));
        EXPECT_TRUE(IsFacingRight(Direction::DownRight));
        EXPECT_TRUE(IsFacingRight(Direction::UpRight));
        EXPECT_TRUE(IsFacingRight(Direction::Up));

        EXPECT_FALSE(IsFacingRight(Direction::Left));
        EXPECT_FALSE(IsFacingRight(Direction::DownLeft));
        EXPECT_FALSE(IsFacingRight(Direction::UpLeft));
        EXPECT_FALSE(IsFacingRight(Direction::Down));
    }

    //Exactly the three upward arcs are drawn from behind, and they are the ones where a held gun goes behind the body
    TEST(FacingHelpers, OnlyTheUpwardArcsFaceBack)
    {
        EXPECT_TRUE(IsFacingBack(Direction::Up));
        EXPECT_TRUE(IsFacingBack(Direction::UpLeft));
        EXPECT_TRUE(IsFacingBack(Direction::UpRight));

        EXPECT_FALSE(IsFacingBack(Direction::Right));
        EXPECT_FALSE(IsFacingBack(Direction::DownRight));
        EXPECT_FALSE(IsFacingBack(Direction::Down));
        EXPECT_FALSE(IsFacingBack(Direction::DownLeft));
        EXPECT_FALSE(IsFacingBack(Direction::Left));
    }

    //The editor can rewrite the arc table live; this is the button that puts a botched edit back
    TEST(DirectionRanges, ResetRestoresTheDefaultArcs)
    {
        DirectionUtils::GetRanges().clear();
        EXPECT_THROW(DirectionUtils::GetDirectionFromAngle(0.f), std::out_of_range);

        DirectionUtils::ResetRangesToDefault();
        EXPECT_EQ(DirectionUtils::GetDirectionFromAngle(0.f), Direction::Right);
        EXPECT_EQ(DirectionUtils::GetRanges().size(), 9u); //eight arcs, and Right written twice
    }
}
