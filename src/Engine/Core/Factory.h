#pragma once
#include <memory>
#include "../../Game/Managers/GameState.h"

class Engine;

//Factory global functions: 
namespace ETG
{
    //Forward declarations
    void RegisterGameObject(GameObjectBase* obj);
    void UnregisterGameObject(GameObjectBase* obj);

    //NOTE: So important. Implemented Factory Method.
    //It's required to first construct the object, afterwards call some functions automatically for all game objects. Calling stuffs in Constructor of the base class will not be applicable for RTTI
    //Because RTTI will only retain base class' metadata. Employing factory method will let me set object name easily based on callee class type name.
    //Force to set Owner to be Scene
    //THe bottom overloaded template function needs to be invoked if GameObject is given. This function should be invoked if first argument only is not GameObject
    //SFIANE needs to be implemented for this purpose. 
    template <typename T, typename... Args>
    std::unique_ptr<T> CreateGameObjectDefault(Args&&... args)
    {
        auto obj = std::make_unique<T>(std::forward<Args>(args)...);
        obj->Owner = GameState::GetInstance().GetSceneObj();
        obj->template SetTypeInfo<T>(); //Set the type ID for this object


        obj->SetObjectNameToSelfClassName();
        RegisterGameObject(obj.get());
        return obj;
    }

    template <typename T, typename... Args>
    std::unique_ptr<T> CreateGameObjectAttached(GameObjectBase* OwnerObj, Args&&... args)
    {
        auto obj = std::make_unique<T>(std::forward<Args>(args)...);
        obj->Owner = OwnerObj;
        obj->template SetTypeInfo<T>(); //Set the type ID for this object

        // After full construction, set the object name based on its true dynamic type name and register it to the scene list
        obj->SetObjectNameToSelfClassName();
        RegisterGameObject(obj.get());
        return obj;
    }

    inline void RegisterGameObject(GameObjectBase* obj)
    {
        GameState::GetInstance().GetSceneObjs().push_back(obj);
    }

    //For now this function is only for updating hierarchy tab for removed game objects.
    inline void UnregisterGameObject(GameObjectBase* obj)
    {
        auto& sceneObjs = GameState::GetInstance().GetSceneObjs();
        std::erase(sceneObjs, obj);
    }

    //NOTE: NOT USED YET. For now I am unsure how this function should be. Game objects always constructed as unique_ptr. Removing them from container will already deallocate the game object. So
    //UnregisterGameObject function will be enough to remove the object from the container (for now).
    template <typename T>
    void DestroyGameObject(std::unique_ptr<T>& obj)
    {
        if (!obj) return; // Safety check

        UnregisterGameObject(obj.get());

        // Reset the unique_ptr to release memory
        obj.reset();
    }
}
