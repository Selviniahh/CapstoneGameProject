#include "../Framework/InteractiveTest.h"
#include "../Framework/InteractiveTestRegistry.h"
#include "../Framework/TestEnvironment.h"
#include "../Framework/TestWatchers.h"
#include "Game/Characters/Hero/Hero.h"
#include "Utils/DirectionUtils.h"
#include "Utils/StrManipulateUtil.h"

//=====================================================================================================================
//  THE 8-WAY FACING TEST - and the smallest complete example of what an interactive test looks like.
//
//  The mechanic: the hero faces the mouse in eight 45 degree arcs (Engine/Core/Direction.h). Whether that actually
//  works cannot be asserted from a unit test - it takes a mouse being swept around a hero on screen - so it is
//  asserted here instead, by watching what the hero does while a human sweeps the mouse.
//=====================================================================================================================

using namespace ETG;
using namespace ETG::Testing;

namespace
{
    class HeroDirectionTest final : public InteractiveTest
    {
    public:
        [[nodiscard]] std::string GetInstructions() const override
        {
            return "Sweep the mouse in a full circle around the hero. The hero turns to face it in eight 45 degree "
                "arcs; the first check passes once every one of the eight has been faced at least once.";
        }

        //--------------------------------------------------------------------------------------------------------
        //  The world this test needs: one hero, nothing else. No enemies, no guns lying around, no items - none of
        //  that has anything to do with facing, and leaving it out is what makes a failure here mean one thing
        //--------------------------------------------------------------------------------------------------------
        void SetUp(TestEnvironment& env) override
        {
            env.SpawnHero({0.f, 0.f});

            //The two assertions. Declared here, resolved in Update. Until then the panel shows them as PENDING,
            //which is exactly the "pending -> true when I turn the character with the mouse" behaviour wanted
            AllEightFaced = AddCheck("Is all directions being switched?",
                                     "Sweep the mouse in a full circle around the hero");

            FacingMatchesAim = AddCheck("Does the facing agree with the aim angle?",
                                        "Same sweep - this one is watching for a mismatch");
        }

        //--------------------------------------------------------------------------------------------------------
        //  One frame. Everything a check needs to know is read out of the live hero
        //--------------------------------------------------------------------------------------------------------
        void Update(TestEnvironment& env) override
        {
            Hero* hero = env.GetHero();
            if (!hero) return; //the world is being rebuilt this frame

            //<---------- Check 1: every arc gets faced ---------->
            Coverage.Observe(hero->CurrentDir);
            AllEightFaced.Progress(Coverage.Describe()); //"3 / 8 seen - missing: Down, Left, ..."
            AllEightFaced.PassIf(Coverage.IsComplete(), "all eight arcs were faced");

            //<---------- Check 2: the facing is the one the aim angle resolves to ---------->
            //The hero's facing and its aim angle are written from the same mouse position, one line apart, so they
            //can only disagree if the arc table (DirectionUtils::GetRanges) has been edited into an inconsistent
            //state - which the editor lets you do, and which this catches the moment it turns the hero the wrong way.
            //
            //A dash owns the hero's facing while it lasts (it faces the dash, not the mouse), so those frames are
            //not evidence of anything and are skipped
            if (hero->GetState() != HeroStateEnum::Dash)
            {
                const Direction expected = DirectionUtils::GetDirectionFromAngle(hero->AimAngle);

                if (expected != hero->CurrentDir)
                {
                    FacingMatchesAim.Fail(std::string("aim ") + std::to_string(static_cast<int>(hero->AimAngle))
                        + " deg resolves to " + EnumToString(expected)
                        + " but the hero is facing " + EnumToString(hero->CurrentDir));
                }
                else
                {
                    //Only worth calling a pass once the sweep has actually covered the circle: agreeing about one
                    //arc proves nothing
                    FacingMatchesAim.Progress("in sync so far");
                    FacingMatchesAim.PassIf(Coverage.IsComplete(), "stayed in sync across all eight arcs");
                }
            }
        }

    private:
        CheckHandle AllEightFaced;
        CheckHandle FacingMatchesAim;

        //The bookkeeping lives in a watcher instead of in this test, because "have all eight been seen" is a
        //question the next facing test will ask too (Test/Interactive/Framework/TestWatchers.h)
        DirectionCoverage Coverage;
    };
}

//The line that makes it appear in the panel: class, category, display name
ETG_INTERACTIVE_TEST(HeroDirectionTest, "Hero", "8-way facing follows the mouse")
