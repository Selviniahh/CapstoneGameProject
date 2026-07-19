#pragma once
#include <cstdint>
#include <string_view>
#include <unordered_set>
#include <typeindex>
#include <unordered_map>

namespace ETG
{
    class TypeID
    {
    public:
        using IDType = uint32_t;

        // Get unique ID for a type
        //NOTE: T might look like not used but based on every given typename, we creating static variable id. So every typename will have a static id.
        //If typename is already defined before, it will return already created id without INCREMENTING.
        //If typename seen for the first time, it will create a new id and return it.
        //In every new typename, id will be  
        template<typename T>
        static IDType GetID() {
            static const IDType id = NextID(); //if this executed before with same typename, this line will be skipped
            return id;
        }

        // Register inheritance relationship
        template<typename Derived, typename Base>
        static void RegisterBaseClass() {
            const IDType derivedID = GetID<Derived>();
            const IDType baseID = GetID<Base>();
            
            // Store this inheritance relationship
            BaseClasses[derivedID].insert(baseID);
        }
        
        // Check if Child inherits from Parent
        static bool IsBaseOf(const IDType childID, const IDType parentID) {
            if (childID == parentID) return true; // Same type
            
            // Check direct inheritance
            const auto it = BaseClasses.find(childID);
            if (it != BaseClasses.end()) {
                if (it->second.contains(parentID)) return true;
                
                // Check recursive inheritance
                for (IDType baseID : it->second) {
                    if (IsBaseOf(baseID, parentID)) return true;
                }
            }
            
            return false;
        }
        
        // Get type name for debugging
        template<typename T>
        static std::string_view GetName() {
            return typeid(T).name();
        }

    private:
        static IDType NextID() {
            static IDType nextID = 0;
            return nextID++;
        }
        
        // Store inheritance relationships (derived -> set of bases)
        inline static std::unordered_map<IDType, std::unordered_set<IDType>> BaseClasses;
    };
}
