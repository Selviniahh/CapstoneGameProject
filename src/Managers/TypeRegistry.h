#pragma once
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <type_traits>
#include <utility>
#include "../Core/GameObjectBase.h"
#include "../UI/UserInterface.h"
#include "../Characters/Hero.h"
#include "../Core/Scene/Scene.h"
#include "../Characters/Components/InputComponent.h"
#include "../Core/Components/BaseMoveComp.h"
#include "../Engine/Reflection.h"
#include "../Characters/Components/HeroMoveComp.h"
#include "../Characters/Components/HeroAnimComp.h"
#include "../Guns/Base/GunBase.h"
#include "../Guns/RogueSpecial/RogueSpecial.h"
#include "../Projectile/ProjectileBase.h"
#include "../Characters/Hand/Hand.h"
#include "../Enemy/EnemyBase.h"
#include "../Enemy/BulletMan/BulletMan.h"

//TODO: This class should be completely automated. A script that will run inside cmake before any executable should read all the classes with have (GAMECLASS) macro top of the name and
//TODO: generate all the RegisterType<ClassName> automatically. All source should be gathered in build directory and every file should have suffix "Gen` For example "HeroGen" just like unreal engine
//TODO: Then instead of taking the source files, cmake should take the files with `Gen` suffix. This way I can avoid calling `RegisterType` for every class and BOOST_DESCRIBE_CLASS.
//TODO: However I am not sure how ninja builder's incremental build will be affected from this. I should calculate checksums of the source files and if the class changed, then I should run the script to create gen files for only changed classes.

//TODO: Another problem: I have to manually call Update and Draw for every class. Handle their draw order etc. In game engines they automated. With another script as long as a game object initialized, it can handle calling Update and Draw automatically every time. 

namespace ETG
{
    class TypeRegistry
    {
    public:
        // Register a specific type for reflection.
        //NOTE: the given T doesn't have to be GameObject or Comp anymore. Utility classes like Animation or AnimManager also can be registered to reflection. 
        template <typename T>
        static void RegisterType()
        {
            // Store the type info for later lookup
            RegisteredTypes[std::type_index(typeid(T))] =
                TypeData{
                    std::type_index(typeid(T)),
                    [](GameClass* obj)
                    {
                        if constexpr (std::is_base_of_v<GameObjectBase, T>)
                        {
                            // Convert obj to be the type of T. T is child of GameObjectBase
                            if (auto* Child = dynamic_cast<T*>(obj))
                            {
                                Reflection::PopulateReflection<T>(*Child);
                                return true;
                            }
                        }
                       
                        return false;
                    }
                };
        }

        //Process an object using its runtime type
        static bool ProcessObject(GameClass* obj)
        {
            if (!obj)
                return false; // Changed from throwing exception to returning false

            // Try direct match first (fastest)
            const auto exactType = std::type_index(typeid(*obj));
            const auto it = RegisteredTypes.find(exactType);
            if (it != RegisteredTypes.end()) //if found
            {
                //After this line executed, if above handler function not delegated, there's a problem with given obj's type. Make sure it's only GameObject not any child of it.
                //Before calling ProcessObject, upcast with dynamic_cast
                return it->second.handler(obj);
            }

            //Default to base class if no match
            Reflection::PopulateReflection<GameClass>(*obj);
            return true;
        }

        struct TypeData
        {
            std::type_index typeIdx;
            std::function<bool(GameClass*)> handler;

            TypeData() : typeIdx(typeid(void)), handler(nullptr)
            {
            }

            TypeData(const std::type_index idx, std::function<bool(GameClass*)> h) : typeIdx(idx), handler(std::move(h))
            {
            }
        };

        static void InitializeTypeRegistry()
        {
            //It doesn't matter to add or not the base class if child is added
            RegisterType<GameClass>();
            RegisterType<GameObjectBase>();
            RegisterType<ComponentBase>();
            RegisterType<Scene>();
            RegisterType<Hero>();
            RegisterType<UserInterface>();
            RegisterType<InputComponent>();
            RegisterType<HeroMoveComp>();
            RegisterType<Animation>();
            RegisterType<HeroAnimComp>();
            RegisterType<GunBase>();
            RegisterType<RogueSpecial>();
            RegisterType<RogueSpecialAnimComp>();
            RegisterType<ProjectileBase>();
            RegisterType<Hand>();
            RegisterType<EnemyBase>();
            RegisterType<BulletMan>();
        }

        static inline std::unordered_map<std::type_index, TypeData> RegisteredTypes;
    };
}
