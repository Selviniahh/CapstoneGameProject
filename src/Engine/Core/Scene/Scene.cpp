#include "Scene.h"

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

    void Scene::PopulateSpecificWidgets()
    {
        GameObjectBase::PopulateSpecificWidgets();

        if (PopulateGameWidgets) PopulateGameWidgets();
    }
}
