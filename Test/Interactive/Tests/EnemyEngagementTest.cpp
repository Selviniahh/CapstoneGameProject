#include <algorithm>
#include <cmath>
#include <cstdio>
#include <imgui.h>
#include "../Framework/InteractiveTest.h"
#include "../Framework/InteractiveTestRegistry.h"
#include "../Framework/TestEnvironment.h"
#include "../Framework/TestWatchers.h"
#include "Engine/Core/Components/BaseHealthComp.h"
#include "Game/Characters/Enemy/BulletMan/BulletMan.h"
#include "Game/Characters/Hero/Hero.h"

//=====================================================================================================================
//  ENEMY ENGAGEMENT - the example of a test whose world has more than one thing in it, and whose checks are about
//  what those things do to each other: does the enemy come after the hero, does shooting it hurt it, does it die.
//
//  Note the ORDER in SetUp. The hero goes first, always: an enemy captures Hero::Get() while it is being
//  constructed, so an enemy built into a heroless world is a null pointer waiting to happen. SpawnEnemy throws a
//  readable error rather than letting that happen.
//=====================================================================================================================

using namespace ETG;
using namespace ETG::Testing;

namespace
{
    class EnemyEngagementTest final : public InteractiveTest
    {
    public:
        [[nodiscard]] std::string GetInstructions() const override
        {
            return "One BulletMan is spawned to the right of the hero. Stand still and it should close in; "
                "shoot it and its health should drop; keep shooting and it should die and leave the world.";
        }

        void SetUp(TestEnvironment& env) override
        {
            Hero* hero = env.SpawnHero({0.f, 0.f});

            //Tuning the hero for the test rather than for the game. The point of a per-test world: this hero
            //cannot be killed while you take your time watching the enemy, and the game's hero is unaffected
            hero->HealthComp->CurrentHealth = 999.f;

            Enemy = env.SpawnEnemy<BulletMan>({SpawnDistance, 0.f});
            StartingHealth = Enemy->HealthComp->CurrentHealth;

            ChasesTheHero = AddCheck("Does the enemy close the distance?",
                                     "Stand still and let it walk towards you");

            TakesDamage = AddCheck("Does shooting it reduce its health?",
                                   "Aim at it and hold left mouse");

            DiesAndLeaves = AddCheck("Does it die and leave the world?",
                                     "Keep shooting until its health is gone");
        }

        void Update(TestEnvironment& env) override
        {
            const Hero* hero = env.GetHero();
            if (!hero) return;

            //<---------- Check 3 first: the enemy may already be gone ---------->
            //Once it is destroyed the pointer is dangling, so nothing below may touch it. GameClass::IsValid is
            //the question to ask about any pointer a test held on to across frames
            if (!GameClass::IsValid(Enemy))
            {
                //An enemy that vanished without ever being hurt is not a passing death test
                DiesAndLeaves.PassIf(TakesDamage.IsPassed(), "the enemy died and was swept out of the world");
                DiesAndLeaves.FailIf(TakesDamage.IsPending(), "the enemy left the world without ever taking damage");
                Enemy = nullptr;
                return;
            }

            //<---------- Check 1: the chase ---------->
            const ETG::Vector2f delta = Enemy->GetPosition() - hero->GetPosition();
            const float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            ClosestSeen = std::min(ClosestSeen, distance);

            char progress[128];
            std::snprintf(progress, sizeof(progress), "%.0f px away now, %.0f px at the closest (spawned at %.0f px)",
                          distance, ClosestSeen, SpawnDistance);
            ChasesTheHero.Progress(progress);

            //"Closed in" means it got meaningfully nearer than where it started, not that it touched the hero -
            //BulletMan stops at its own attack distance and shoots from there
            ChasesTheHero.PassIf(ClosestSeen <= SpawnDistance - RequiredApproach, progress);

            //<---------- Check 2: damage lands ---------->
            const float health = Enemy->HealthComp->CurrentHealth;

            char healthText[128];
            std::snprintf(healthText, sizeof(healthText), "health %.1f of %.1f", health, StartingHealth);
            TakesDamage.Progress(healthText);
            TakesDamage.PassIf(health < StartingHealth, healthText);

            //A health bar that goes UP on its own is a bug worth catching for free while we are here
            TakesDamage.FailIf(health > StartingHealth + 0.01f, "the enemy healed itself");

            DiesAndLeaves.Progress(health <= 0.f ? "dead, waiting for it to be swept out" : healthText);
        }

        void DrawWidgets(TestEnvironment& env) override
        {
            //Spawning more of them mid-test is exactly the kind of thing a test's own widgets are for
            if (ImGui::Button("Spawn another BulletMan"))
                env.SpawnEnemy<BulletMan>({SpawnDistance, 60.f});

            ImGui::TextDisabled("BulletMen in the world: %zu", env.CountOf<BulletMan>());
        }

    private:
        CheckHandle ChasesTheHero;
        CheckHandle TakesDamage;
        CheckHandle DiesAndLeaves;

        //The one the checks are about. The extra ones spawned from the widget above are not tracked on purpose:
        //a check has to be about a thing you can name
        BulletMan* Enemy{nullptr};

        float StartingHealth{0.f};
        float ClosestSeen{1e9f};

        static constexpr float SpawnDistance = 140.f; //where the enemy starts, px to the right of the hero
        static constexpr float RequiredApproach = 40.f; //how much nearer it has to get for the chase to count
    };
}

ETG_INTERACTIVE_TEST(EnemyEngagementTest, "Enemies", "BulletMan chases, takes damage and dies")
