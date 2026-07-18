#include "Scene.h"
#include "../../Enemy/BulletMan/BulletMan.h"
#include "../../Managers/GameManager.h"
#include "../../Managers/GameState.h"
#include <imgui.h>

namespace ETG
{
    Scene::Scene()
    {
        IsGameObjectUISpecified = true;
    }

    void Scene::Initialize()
    {
        GameObjectBase::Initialize();
        IsGameObjectUISpecified = true;

    }

    void Scene::Update()
    {
        GameObjectBase::Update();
    }

    void Scene::Draw()
    {
        //Scene has no visual of its own. Spawned objects live in GameManager's central list and draw themselves.
    }

    void Scene::SpawnBulletMan(float x, float y)
    {
        //Ownership lives in GameManager's central scene list; Scene only requests the spawn
        GameState::GetInstance().GetGameManager()->SpawnGameObject<BulletMan>(ETG::Vector2f{x, y});
    }

    void Scene::PopulateSpecificWidgets()
    {
        GameObjectBase::PopulateSpecificWidgets();

        // Spawn button
        if (ImGui::Button("Spawn BulletMan"))
        {
            SpawnBulletMan(spawnX, spawnY);
        }

        // Display count of active enemies
        int enemyCount = 0;
        for (const auto* obj : GameState::GetInstance().GetOrderedSceneObjs())
        {
            // “Nesne geçerliyse ve obj gerçekten BulletMan ise…”
            if (GameClass::IsValid(obj) && dynamic_cast<const BulletMan*>(obj))
                enemyCount++;
        }
        ImGui::Text("Active enemies: %d", enemyCount);
    }
}
