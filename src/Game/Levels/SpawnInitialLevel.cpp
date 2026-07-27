#include "SpawnInitialLevel.h"
#include <imgui.h>
#include "../Managers/GameManager.h"
#include "../Characters/Hero.h"
#include "../Guns/AK-47/AK47.h"
#include "../Guns/SawedOff/SawedOff.h"
#include "../Guns/Magnum/Magnum.h"
#include "../Enemy/BulletMan/BulletMan.h"
#include "../Items/Active/DoubleShoot.h"
#include "../Items/Active/TakeNoDamage.h"
#include "../Items/Passive/PlatinumBullets.h"

void ETG::SpawnInitialLevel::Spawn(GameManager& game)
{
    game.SpawnGameObject<Hero>(Vector2f{10, 10});
    game.SpawnGameObject<AK47>(Vector2f{-100, 100});
    game.SpawnGameObject<SawedOff>(Vector2f{-150, 100});
    game.SpawnGameObject<Magnum>(Vector2f{-200, 100});
    game.SpawnGameObject<BulletMan>(Vector2f{50, 50});
    game.SpawnGameObject<PlatinumBullets>();
    game.SpawnGameObject<DoubleShoot>();
    game.SpawnGameObject<TakeNoDamage>();

    //Game-specific editor widgets on the Scene panel (the engine's Scene only exposes the hook)
    Scene::Get()->PopulateGameWidgets = [&game]
    {
        static float spawnPos[2] = {0.f, 0.f};
        ImGui::InputFloat2("Spawn Pos", spawnPos);

        if (ImGui::Button("Spawn BulletMan"))
        {
            game.SpawnGameObject<BulletMan>(Vector2f{spawnPos[0], spawnPos[1]});
        }

        // Display count of active enemies
        int enemyCount = 0;
        for (const auto* obj : Scene::Get()->SceneObjs)
        {
            if (GameClass::IsValid(obj) && dynamic_cast<const BulletMan*>(obj))
                enemyCount++;
        }
        ImGui::Text("Active enemies: %d", enemyCount);
    };
}
