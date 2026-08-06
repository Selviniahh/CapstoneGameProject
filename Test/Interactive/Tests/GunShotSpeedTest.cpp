#include <cstdio>
#include <imgui.h>
#include "../Framework/InteractiveTest.h"
#include "../Framework/InteractiveTestRegistry.h"
#include "../Framework/TestEnvironment.h"
#include "../Framework/TestWatchers.h"
//The gun's destructor is instantiated here (env.GiveHeroGun<AK47>() builds one), and it destroys a
//unique_ptr<CollisionComponent> - so the component's definition has to be visible, not just its declaration
#include "Engine/Core/Components/CollisionComponent.h"
#include "Game/Characters/Hero/Hero.h"
#include "Game/Guns/AK-47/AK47.h"
#include "Game/Guns/Base/GunBase.h"
#include "Game/Projectile/ProjectileBase.h"

//=====================================================================================================================
//  BULLET SPEED - "did the bullet really travel ShotSpeed pixels per second for three seconds?"
//
//  This is the measured kind of interactive test: the human only has to pull the trigger, and the numbers decide the
//  verdict. It is also the example of a test that RETUNES the world it is testing - the bullet's range is pushed out
//  so it survives the measurement window, and it is pushed out through a stat modifier keyed by this test, not by
//  overwriting the number the gun was authored with.
//=====================================================================================================================

using namespace ETG;
using namespace ETG::Testing;

namespace
{
    //Keyed removal: a StatModifier is detached by the same source string that attached it, so this test can undo
    //exactly its own change and nothing else's
    constexpr const char* ModifierSource = "GunShotSpeedTest";

    class GunShotSpeedTest final : public InteractiveTest
    {
    public:
        [[nodiscard]] std::string GetInstructions() const override
        {
            return "Aim away from the hero and hold left mouse. The first bullet that appears is tracked for the "
                "measurement window below, then its travel is compared against the gun's ShotSpeed.";
        }

        //--------------------------------------------------------------------------------------------------------
        //  World: a hero holding an AK, and nothing for the bullet to hit
        //--------------------------------------------------------------------------------------------------------
        void SetUp(TestEnvironment& env) override
        {
            env.SpawnHero({0.f, 0.f});

            //Spawn a gun and put it straight in the hero's hands. This is the "change my character for this test"
            //move: no walking over to a pickup, no editing the game's level
            Gun = env.GiveHeroGun<AK47>();

            //A bullet dies once it has flown its Range, which is a lot shorter than three seconds of flight. The
            //range is stretched with a modifier under this test's name: the gun's authored value is untouched, and
            //TearDown takes the stretch back off
            Gun->Range.AddModifier(ModifierSource, StatOp::Flat, 100000.f);

            //Firing faster than the AK is authored to, so the human is not left waiting on the fire rate. Same
            //keyed-modifier story as the range above: base value untouched, removed by name in TearDown
            Gun->FireRate.AddModifier(ModifierSource, StatOp::Flat, -0.4f);

            TravelledAsFast = AddCheck("Does the bullet travel at the gun's ShotSpeed?",
                                       "Hold left mouse and let one bullet fly for the whole window");

            FlewStraight = AddCheck("Does it fly in a straight line?",
                                    "Same shot - the path walked and the distance covered must match");
        }

        //--------------------------------------------------------------------------------------------------------
        //  Undo what was done to something that outlives the test. The gun is destroyed with the rest of this
        //  test's world a moment later, so this is not strictly necessary - it is here because it is the pattern
        //  to copy when a test modifies something it did NOT spawn
        //--------------------------------------------------------------------------------------------------------
        void TearDown(TestEnvironment& env) override
        {
            if (GameClass::IsValid(Gun)) Gun->RemoveAllModifiersFrom(ModifierSource);
            Gun = nullptr;
        }

        void Update(TestEnvironment& env) override
        {
            if (!GameClass::IsValid(Gun)) return;

            //<---------- Latch onto the first bullet that exists ---------->
            //The gun creates its own projectiles, so the test does not spawn them - it looks for them. Start() is
            //a no-op once a measurement is running, so this is safe to call every frame
            if (!Probe.HasStarted())
            {
                if (ProjectileBase* bullet = env.FindFirst<ProjectileBase>())
                {
                    Probe.Start(bullet);

                    //Read the speed at the moment of firing: an item picked up mid-flight must not move the
                    //goalposts of a measurement already under way
                    ExpectedSpeed = Gun->ShotSpeed;
                }

                TravelledAsFast.Progress("waiting for the first shot");
                return;
            }

            Probe.Tick();

            //<---------- The bullet died before the window was up ---------->
            //It hit something, or it ran out of range anyway. Either way there is no measurement to report, and
            //saying so is far more useful than reporting the truncated distance as a failure
            if (Probe.TargetDiedEarly() && TravelledAsFast.IsPending())
            {
                TravelledAsFast.Fail("the bullet was destroyed after " + std::to_string(Probe.GetSeconds())
                    + " s - nothing may be in its way, and its range has to outlast the window");
                return;
            }

            //Progress text with the real numbers in it, so the panel is watchable while the bullet is in flight
            char progress[160];
            std::snprintf(progress, sizeof(progress), "%.2f s of %.2f s - %.0f px so far (%.0f px/s)",
                          Probe.GetSeconds(), MeasureSeconds, Probe.GetDisplacement(), Probe.GetAverageSpeed());
            TravelledAsFast.Progress(progress);

            //<---------- The window is up: compare ---------->
            if (Probe.GetSeconds() >= MeasureSeconds)
            {
                const float expectedDistance = ExpectedSpeed * Probe.GetSeconds();

                //A frame's worth of slack plus a percentage: the probe samples once per frame, so it can never be
                //more accurate than one frame of travel, and that is the floor the tolerance has to respect
                const float tolerance = expectedDistance * TolerancePercent * 0.01f + ExpectedSpeed * 0.05f;

                TravelledAsFast.ExpectNear(Probe.GetDisplacement(), expectedDistance, tolerance, " px");

                //A straight flight walks exactly as far as it gets from where it started. Anything else means the
                //bullet curved, which is a different bug from it being slow
                FlewStraight.ExpectNear(Probe.GetPathLength(), Probe.GetDisplacement(), tolerance, " px");

                Probe.Stop();
            }
        }

        //--------------------------------------------------------------------------------------------------------
        //  The test's own knobs. Anything a test wants to poke at while it runs goes here
        //--------------------------------------------------------------------------------------------------------
        void DrawWidgets(TestEnvironment& env) override
        {
            ImGui::SliderFloat("Measurement window (s)", &MeasureSeconds, 0.25f, 5.f, "%.2f");
            ImGui::SliderFloat("Tolerance (%)", &TolerancePercent, 1.f, 25.f, "%.0f");

            if (ImGui::Button("Measure the next bullet again"))
            {
                Probe.Reset();
                ResetAllChecks();
            }

            if (GameClass::IsValid(Gun))
                ImGui::TextDisabled("ShotSpeed %.0f px/s | Range %.0f px", static_cast<float>(Gun->ShotSpeed), static_cast<float>(Gun->Range));
        }

    private:
        CheckHandle TravelledAsFast;
        CheckHandle FlewStraight;

        TravelProbe Probe;
        GunBase* Gun{nullptr};

        float ExpectedSpeed{0.f};

        //Three seconds by default, which is the window this test was written for
        float MeasureSeconds{3.f};
        float TolerancePercent{5.f};
    };
}

ETG_INTERACTIVE_TEST(GunShotSpeedTest, "Guns", "Bullet travels at the gun's ShotSpeed")
