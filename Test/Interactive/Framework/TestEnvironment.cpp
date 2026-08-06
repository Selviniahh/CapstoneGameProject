#include "TestEnvironment.h"
#include <stdexcept>
#include "Engine/Managers/Time.h"
#include "Game/Characters/Hero/Hero.h"
#include "Game/Guns/Base/GunBase.h"

namespace ETG::Testing
{
    Hero* TestEnvironment::SpawnHero(const ETG::Vector2f& position)
    {
        HeroPtr = Spawn<Hero>(position);
        return HeroPtr;
    }

    GunBase* TestEnvironment::GetHeroGun() const
    {
        return HeroPtr ? HeroPtr->GetCurrentHoldingGun() : nullptr;
    }

    void TestEnvironment::EquipOnHero(GunBase* gun) const
    {
        if (HeroPtr && gun) HeroPtr->EquipGun(gun);
    }

    void TestEnvironment::RequireHero(const char* caller) const
    {
        if (HeroPtr) return;

        //A hard error rather than a silent null: this is a mistake in the test's SetUp, and the message says
        //exactly how to fix it
        throw std::runtime_error(std::string(caller) + " needs a hero in the world. "
                                 "Call env.SpawnHero() first in your test's SetUp().");
    }

    float TestEnvironment::DeltaSeconds()
    {
        return Time::FrameTick;
    }

    void TestEnvironment::AdvanceClock()
    {
        Elapsed += Time::FrameTick;
    }

    void TestEnvironment::DestroyEverythingSpawned()
    {
        //Reverse order, so a gun goes before the hero holding it. Nothing depends on it today - the sweep runs
        //once for the whole batch - but it keeps the teardown order the mirror image of the setup order
        for (auto it = Spawned.rbegin(); it != Spawned.rend(); ++it)
        {
            if (GameClass::IsValid(*it))
                (*it)->MarkForDestroy();
        }

        Spawned.clear();
        HeroPtr = nullptr;
        Elapsed = 0.f;
    }
}
