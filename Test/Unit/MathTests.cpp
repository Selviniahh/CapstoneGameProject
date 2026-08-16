#include <gtest/gtest.h>
#include <stdexcept>
#include "Utils/Math.h"

//=====================================================================================================================
//  UNIT TESTS for the shared maths helpers. Small, boring, and worth having: the gun rig, the projectiles and every
//  timed effect in the game are built out of these four or five functions, so a sign flip here is a sign flip
//  everywhere at once.
//=====================================================================================================================

namespace
{
    TEST(MathVector, NormalizeReturnsAUnitVectorInTheSameDirection)
    {
        const ETG::Vector2f normalized = Math::Normalize(ETG::Vector2f{3.f, 4.f});

        EXPECT_FLOAT_EQ(Math::Length(normalized), 1.f);
        EXPECT_FLOAT_EQ(normalized.x, 0.6f);
        EXPECT_FLOAT_EQ(normalized.y, 0.8f);
    }

    //A zero vector has no direction, so there is no sane answer to return - it throws rather than handing back a NaN
    //that would then spread through a position
    TEST(MathVector, NormalizeRejectsTheZeroVector)
    {
        EXPECT_THROW(Math::Normalize(ETG::Vector2f{0.f, 0.f}), std::runtime_error);
    }

    TEST(MathVector, LengthMatchesPythagoras)
    {
        EXPECT_FLOAT_EQ(Math::Length({3.f, 4.f}), 5.f);
        EXPECT_FLOAT_EQ(Math::VectorLength(ETG::Vector2f{5.f, 12.f}), 13.f);
        EXPECT_FLOAT_EQ(Math::VectorSizeSquared(ETG::Vector2f{3.f, 4.f}), 25.f);
    }

    TEST(MathAngles, DegreesAndRadiansRoundTrip)
    {
        EXPECT_NEAR(Math::RadiansToDegrees(Math::AngleToRadian(37.f)), 37.f, 1e-4f);
        EXPECT_NEAR(Math::AngleToRadian(180.f), std::numbers::pi_v<float>, 1e-5f);
    }

    //0 degrees is straight right and the angle grows clockwise on screen, because +y points down
    TEST(MathAngles, RadianToDirectionUsesScreenSpace)
    {
        const ETG::Vector2f right = Math::RadianToDirection(Math::AngleToRadian(0.f));
        EXPECT_NEAR(right.x, 1.f, 1e-5f);
        EXPECT_NEAR(right.y, 0.f, 1e-5f);

        const ETG::Vector2f down = Math::RadianToDirection(Math::AngleToRadian(90.f));
        EXPECT_NEAR(down.x, 0.f, 1e-5f);
        EXPECT_NEAR(down.y, 1.f, 1e-5f);
    }

    //What the held-gun rig turns an authored offset into. Rotating a right-pointing offset by 90 degrees has to put
    //it below the anchor, not above it
    TEST(MathAngles, RotateVectorTurnsClockwiseOnScreen)
    {
        const ETG::Vector2f rotated = Math::RotateVector(90.f, {1.f, 1.f}, {10.f, 0.f});

        EXPECT_NEAR(rotated.x, 0.f, 1e-4f);
        EXPECT_NEAR(rotated.y, 10.f, 1e-4f);
    }

    //Scale is applied before the rotation, which is what mirrors a held gun's offsets when it changes hands
    TEST(MathAngles, RotateVectorAppliesScaleFirst)
    {
        const ETG::Vector2f mirrored = Math::RotateVector(0.f, {-1.f, 1.f}, {10.f, 5.f});

        EXPECT_FLOAT_EQ(mirrored.x, -10.f);
        EXPECT_FLOAT_EQ(mirrored.y, 5.f);
    }

    TEST(MathProjection, DotIsSignedByTheAngleBetween)
    {
        EXPECT_FLOAT_EQ(Math::Dot({1.f, 0.f}, {1.f, 0.f}), 1.f); //ayni yon
        EXPECT_FLOAT_EQ(Math::Dot({1.f, 0.f}, {0.f, 1.f}), 0.f); //dik - kaymanin butun temeli bu
        EXPECT_FLOAT_EQ(Math::Dot({1.f, 0.f}, {-1.f, 0.f}), -1.f); //ters yon
        EXPECT_FLOAT_EQ(Math::Dot({3.f, 4.f}, {2.f, 1.f}), 10.f);
    }

    //v_kayma = v - (v . A)A. Normali sana dogru asagi bakan bir duvarin icine yukari yurumek: yukari giden yari
    //siliniyor, yana giden yariya dokunulmuyor. Duvar boyunca kosmak dedigimiz sey bundan ibaret
    TEST(MathProjection, SlideKeepsTheTangentAndDropsTheNormal)
    {
        const ETG::Vector2f slid = Math::SlideAlongSurface({100.f, -140.f}, {0.f, -1.f});

        EXPECT_FLOAT_EQ(slid.x, 100.f);
        EXPECT_FLOAT_EQ(slid.y, 0.f);
    }

    //Yuzeyin icine hic yonelmemis bir hareketin kaybedecegi sey yoktur - yuzey boyunca hangi yone gidiyor olursa olsun
    TEST(MathProjection, SlideLeavesMotionAlongTheSurfaceAlone)
    {
        const ETG::Vector2f alongLeft = Math::SlideAlongSurface({-70.f, 0.f}, {0.f, -1.f});
        EXPECT_FLOAT_EQ(alongLeft.x, -70.f);
        EXPECT_FLOAT_EQ(alongLeft.y, 0.f);

        //Duvara dosdogru girmekten ise geriye hicbir sey kalmaz - tegeti olmayan bir kayma, durmaktir
        const ETG::Vector2f headOn = Math::SlideAlongSurface({0.f, -200.f}, {0.f, -1.f});
        EXPECT_FLOAT_EQ(headOn.x, 0.f);
        EXPECT_FLOAT_EQ(headOn.y, 0.f);
    }

    //Sonuc, TANIMI GEREGI normale diktir; ve bu, egik bir yuzeyde de gecerlidir - projeksiyonun "bloklanan ekseni
    //sifirla" diye degil de formul olarak yazilmasinin sebebi tam olarak budur
    TEST(MathProjection, SlideResultIsPerpendicularToTheNormalOnADiagonalSurface)
    {
        const ETG::Vector2f diagonal = Math::Normalize(ETG::Vector2f{1.f, -1.f});
        const ETG::Vector2f slid = Math::SlideAlongSurface({0.f, -300.f}, diagonal);

        EXPECT_NEAR(Math::Dot(slid, diagonal), 0.f, 1e-3f);

        //Ve gercekten yol vermis: 45 derecelik bir yuze karsi dosdogru yukari giden 300, her iki yone yarisi olarak cikiyor
        EXPECT_NEAR(slid.x, -150.f, 1e-3f);
        EXPECT_NEAR(slid.y, -150.f, 1e-3f);
    }

    //One shot progressions (reload, cooldown, force falloff): saturates at both ends and stays done once done
    TEST(MathTiming, Progress01Saturates)
    {
        EXPECT_FLOAT_EQ(Math::Progress01(0.f, 2.f), 0.f);
        EXPECT_FLOAT_EQ(Math::Progress01(1.f, 2.f), 0.5f);
        EXPECT_FLOAT_EQ(Math::Progress01(5.f, 2.f), 1.f);

        //A zero length progression is already over, rather than a division by zero
        EXPECT_FLOAT_EQ(Math::Progress01(0.f, 0.f), 1.f);
    }

    //Looping effects (blinking, idle bob): wraps instead of saturating, and never returns a negative
    TEST(MathTiming, Repeat01Wraps)
    {
        EXPECT_FLOAT_EQ(Math::Repeat01(0.25f), 0.25f);
        EXPECT_FLOAT_EQ(Math::Repeat01(1.25f), 0.25f);
        EXPECT_FLOAT_EQ(Math::Repeat01(-0.25f), 0.75f);
    }

    TEST(MathTiming, IntervalLerpWalksFromAToB)
    {
        EXPECT_FLOAT_EQ(Math::IntervalLerp(0.f, 100.f, 10.f, 0.f), 0.f);
        EXPECT_FLOAT_EQ(Math::IntervalLerp(0.f, 100.f, 10.f, 5.f), 50.f);
        EXPECT_FLOAT_EQ(Math::IntervalLerp(0.f, 100.f, 10.f, 20.f), 100.f); //clamped, not overshooting
    }

    TEST(MathMisc, IsInRangeIsInclusiveAtBothEnds)
    {
        EXPECT_TRUE(Math::IsInRange(5.f, 0.f, 10.f));
        EXPECT_TRUE(Math::IsInRange(0.f, 0.f, 10.f));
        EXPECT_TRUE(Math::IsInRange(10.f, 0.f, 10.f));
        EXPECT_FALSE(Math::IsInRange(10.1f, 0.f, 10.f));
    }

    //Bullet spread leans on this, so "never outside the range asked for" is the property that matters
    TEST(MathMisc, RandomNumbersStayInsideTheirRange)
    {
        for (int i = 0; i < 200; ++i)
        {
            const float value = Math::GenRandomNumber(-3.f, 3.f);
            EXPECT_GE(value, -3.f);
            EXPECT_LE(value, 3.f);

            const int integer = Math::GenRandomNumber(1, 6);
            EXPECT_GE(integer, 1);
            EXPECT_LE(integer, 6);
        }
    }
}
