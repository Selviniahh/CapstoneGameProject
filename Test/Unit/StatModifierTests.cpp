#include <gtest/gtest.h>
#include "Engine/Core/Stats/StatModifier.h"

//=====================================================================================================================
//  UNIT TESTS for StatModifier - the number a designer sets and items modify without the two overwriting each other.
//
//      final = (Base + sum of Flat) * product of (1 + Percent)
//
//  The property worth testing is not the formula, it is that REMOVAL IS EXACT: taking a modifier off gives back
//  precisely the value you would have had if it had never been applied, whatever else is attached. That is what
//  lets an item be dropped, and it is why percentages compound instead of summing.
//=====================================================================================================================

using namespace ETG;

namespace
{
    TEST(StatModifier, BareNumberIsTheBaseValue)
    {
        StatModifier speed = 100.f; //implicit conversion, so a stat is written exactly like the float it replaced
        EXPECT_FLOAT_EQ(speed.Get(), 100.f);
        EXPECT_FLOAT_EQ(speed.GetBase(), 100.f);

        //Assigning a number sets the BASE - it does not fight with the modifiers. This is the tuning path
        speed = 250.f;
        EXPECT_FLOAT_EQ(speed, 250.f); //and it converts back to float implicitly too
    }

    TEST(StatModifier, FlatModifiersAddToTheBase)
    {
        StatModifier damage = 10.f;
        damage.AddModifier("PlatinumBullets", StatOp::Flat, 5.f);
        damage.AddModifier("Scope", StatOp::Flat, 2.5f);

        EXPECT_FLOAT_EQ(damage.Get(), 17.5f);
        EXPECT_FLOAT_EQ(damage.GetBase(), 10.f); //the authored value is never touched
    }

    TEST(StatModifier, PercentModifiersScaleTheResult)
    {
        StatModifier fireRate = 100.f;
        fireRate.AddModifier("Item", StatOp::Percent, 0.20f); //+20%

        EXPECT_FLOAT_EQ(fireRate.Get(), 120.f);
    }

    //Compounding, not summing: two +20% items give +44%, not +40%. Summed percentages cannot be removed one at a
    //time without changing what the other one meant, which is the whole reason for this choice
    TEST(StatModifier, PercentModifiersCompound)
    {
        StatModifier value = 100.f;
        value.AddModifier("A", StatOp::Percent, 0.20f);
        value.AddModifier("B", StatOp::Percent, 0.20f);

        EXPECT_FLOAT_EQ(value.Get(), 144.f);
    }

    TEST(StatModifier, FlatAppliesBeforePercent)
    {
        StatModifier value = 100.f;
        value.AddModifier("Flat", StatOp::Flat, 100.f);      //-> 200
        value.AddModifier("Percent", StatOp::Percent, 0.5f); //-> 300

        EXPECT_FLOAT_EQ(value.Get(), 300.f);
    }

    //THE property. Whatever is attached, dropping one item restores the exact value from before it was picked up
    TEST(StatModifier, RemovalIsExact)
    {
        StatModifier value = 80.f;
        value.AddModifier("Keeper", StatOp::Flat, 20.f);
        value.AddModifier("Keeper", StatOp::Percent, 0.25f);

        const float beforePickup = value.Get();

        value.AddModifier("Dropped", StatOp::Flat, 40.f);
        value.AddModifier("Dropped", StatOp::Percent, 0.10f);
        EXPECT_NE(value.Get(), beforePickup);

        EXPECT_TRUE(value.RemoveModifiersFrom("Dropped"));
        EXPECT_FLOAT_EQ(value.Get(), beforePickup);

        //Removing something that was never applied is not an error, it just did nothing
        EXPECT_FALSE(value.RemoveModifiersFrom("NeverApplied"));
    }

    //An item re-applying itself every frame (or after an editor tweak) must not stack with itself: the same
    //source replaces its own modifier instead of adding a second one
    TEST(StatModifier, SameSourceReplacesItselfInsteadOfStacking)
    {
        StatModifier value = 100.f;

        for (int frame = 0; frame < 100; ++frame)
            value.AddModifier("PlatinumBullets", StatOp::Flat, 10.f);

        EXPECT_FLOAT_EQ(value.Get(), 110.f);
        EXPECT_EQ(value.GetModifiers().size(), 1u);
    }

    TEST(StatModifier, TracksWhichSourcesAreAttached)
    {
        StatModifier value = 1.f;
        EXPECT_FALSE(value.HasModifierFrom("Item"));

        value.AddModifier("Item", StatOp::Flat, 1.f);
        EXPECT_TRUE(value.HasModifierFrom("Item"));

        value.ClearModifiers();
        EXPECT_FALSE(value.HasModifierFrom("Item"));
        EXPECT_FLOAT_EQ(value.Get(), 1.f);
    }

    //Changing the base under attached modifiers keeps them attached, and re-folds them onto the new base
    TEST(StatModifier, ChangingTheBaseKeepsModifiers)
    {
        StatModifier value = 100.f;
        value.AddModifier("Item", StatOp::Percent, 0.5f);
        EXPECT_FLOAT_EQ(value.Get(), 150.f);

        value.SetBase(200.f);
        EXPECT_FLOAT_EQ(value.Get(), 300.f);
    }

    //Magazine sizes and ammo counts are conceptually counts; GetInt is what the guns read them through
    TEST(StatModifier, GetIntForStatsThatAreCounts)
    {
        StatModifier magazine = 30.f;
        magazine.AddModifier("Item", StatOp::Percent, 0.10f); //33.0

        EXPECT_EQ(magazine.GetInt(), 33);
    }
}
