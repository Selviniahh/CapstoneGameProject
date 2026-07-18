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
#include "../Core/Components/ArrowComp.h"
#include "../Guns/VFX/MuzzleFlash.h"
#include "../UI/UIObjects/AmmoBarUI.h"
#include "../UI/UIObjects/AmmoIndicatorsUI.h"
#include "../UI/UIObjects/AmmoCounter.h"
#include "../UI/UIObjects/ReloadSlider.h"
#include "../Core/Components/CollisionComponent.h"
#include "../Items/Active/DoubleShoot.h"
#include "../Items/Passive/PlatinumBullets.h"
#include "../Guns/AK-47/AK47.h"
#include "../Guns/Magnum/Magnum.h"
#include "../Guns/SawedOff/SawedOff.h"
#include "../Enemy/BulletMan/BulletMan.h"
#include "../Enemy/BulletMan/Components/BulletManAnimComp.h"
#include "../Core/TypeID.h"

#define REGISTER_BASE_CLASS(Derived, Base) \
TypeID::RegisterBaseClass<Derived, Base>();


//TODO: This class should be completely automated. A pre build script that will run inside cmake before any executable should read all the classes with have (GAMECLASS) macro top of the name and
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
                //Before calling ProcessObject, upcast with dynamic_cast.md
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
            RegisterType<GameClass>();
            RegisterType<GameObjectBase>();
            RegisterType<ComponentBase>();
            REGISTER_BASE_CLASS(ComponentBase, GameObjectBase);
            RegisterType<Scene>();
            REGISTER_BASE_CLASS(Scene, GameObjectBase);
            RegisterType<Hero>();
            REGISTER_BASE_CLASS(Hero, GameObjectBase);
            RegisterType<UserInterface>();
            REGISTER_BASE_CLASS(UserInterface, GameObjectBase);
            RegisterType<InputComponent>();
            REGISTER_BASE_CLASS(InputComponent, ComponentBase);
            RegisterType<HeroMoveComp>();
            REGISTER_BASE_CLASS(HeroMoveComp, BaseMoveComp);
            RegisterType<Animation>();
            REGISTER_BASE_CLASS(Animation, GameClass);
            RegisterType<HeroAnimComp>();
            REGISTER_BASE_CLASS(HeroAnimComp, BaseAnimComp<HeroStateEnum>);
            RegisterType<GunBase>();
            REGISTER_BASE_CLASS(GunBase, GameObjectBase);
            RegisterType<RogueSpecial>();
            REGISTER_BASE_CLASS(RogueSpecial, GunBase);
            RegisterType<RogueSpecialAnimComp>();
            REGISTER_BASE_CLASS(RogueSpecialAnimComp, BaseAnimComp<GunStateEnum>);
            RegisterType<ProjectileBase>();
            REGISTER_BASE_CLASS(ProjectileBase, GameObjectBase);
            RegisterType<Hand>();
            REGISTER_BASE_CLASS(Hand, GameObjectBase);
            RegisterType<ArrowComp>();
            REGISTER_BASE_CLASS(ArrowComp, ComponentBase);
            RegisterType<MuzzleFlash>();
            REGISTER_BASE_CLASS(MuzzleFlash, GameObjectBase);
            RegisterType<AmmoBarUI>();
            REGISTER_BASE_CLASS(AmmoBarUI, GameObjectBase);
            RegisterType<AmmoIndicatorsUI>();
            REGISTER_BASE_CLASS(AmmoIndicatorsUI, Hero);
            RegisterType<AmmoCounter>();
            REGISTER_BASE_CLASS(AmmoCounter, GameObjectBase);
            RegisterType<ReloadSlider>();
            REGISTER_BASE_CLASS(ReloadSlider, GameObjectBase);
            RegisterType<CollisionComponent>();
            REGISTER_BASE_CLASS(CollisionComponent, ComponentBase);
            RegisterType<ActiveItemBase>();
            REGISTER_BASE_CLASS(ActiveItemBase, GameObjectBase);
            RegisterType<DoubleShoot>();
            REGISTER_BASE_CLASS(DoubleShoot, ActiveItemBase);
            RegisterType<PassiveItemBase>();
            REGISTER_BASE_CLASS(PassiveItemBase, GameObjectBase);
            RegisterType<PlatinumBullets>();
            REGISTER_BASE_CLASS(PlatinumBullets, PassiveItemBase);
            RegisterType<AK47>();
            REGISTER_BASE_CLASS(AK47, GunBase);
            RegisterType<SawedOff>();
            REGISTER_BASE_CLASS(SawedOff, GunBase);
            RegisterType<Magnum>();
            REGISTER_BASE_CLASS(Magnum, GunBase);
            RegisterType<SawedOffAnimComp>();
            REGISTER_BASE_CLASS(SawedOffAnimComp, BaseAnimComp<GunStateEnum>);
            RegisterType<BulletMan>();
            REGISTER_BASE_CLASS(BulletMan, EnemyBase);
            RegisterType<BulletManAnimComp>();
            REGISTER_BASE_CLASS(BulletManAnimComp, BaseAnimComp<EnemyStateEnum>);
            RegisterType<EnemyMoveCompBase>();
            REGISTER_BASE_CLASS(EnemyMoveCompBase, BaseMoveComp);
        }

        static inline std::unordered_map<std::type_index, TypeData> RegisteredTypes;
    };
}
