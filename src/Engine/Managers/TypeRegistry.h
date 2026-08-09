#pragma once
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <type_traits>
#include <utility>
#include "../Core/GameObjectBase.h"
#include "../Editor/Reflection.h"
#include "../Editor/DebugPointViz.h"
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
    //Type registration machinery only. The actual list of game types lives on the game side
    //(Game/Managers/RegisterGameTypes) so the engine never has to include game headers.
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

            //Whatever members the walk below turns up, they belong to this object. The Vector2f widget reads this
            //to know which frame an authored point is expressed in; the pop has to happen on every exit path,
            //because ShowImGuiWidget<GameObjectBase*> recurses back into here for nested objects.
            const OwnerScope ownerScope{dynamic_cast<GameObjectBase*>(obj)};

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

        static inline std::unordered_map<std::type_index, TypeData> RegisteredTypes;

    private:
        //ProcessObject has several returns and the reflection walk in between can throw; a scope guard is what
        //keeps the owner stack balanced without repeating the pop on each exit
        struct OwnerScope
        {
            explicit OwnerScope(GameObjectBase* owner) { DebugPointViz::PushOwner(owner); }
            ~OwnerScope() { DebugPointViz::PopOwner(); }

            OwnerScope(const OwnerScope&) = delete;
            OwnerScope& operator=(const OwnerScope&) = delete;
        };
    };
}
