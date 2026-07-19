#pragma once
#include <string>
#include <memory>
#include <vector>
#include <algorithm>

namespace ETG
{
    //Modtype can only be 
    template<typename ModType>
    class ModifierManager
    {
    public:
        //Add a modifier to the gun
        void AddModifier(const std::shared_ptr<ModType>& modifier)
        {
            //Remove any existing modifier of same type
            //Probably I will remove all the modifiers when adding one, bcs only 1 Modifier can exist only and none of modifier can exist at the same time 
            RemoveModifier(modifier->GetModifierName());
            modifiers.push_back(modifier);
        }

        //Remove a modifier by name
        void RemoveModifier(const std::string& modifierName)
        {
            //Better and cooler way to remove from an array instead of traditional for loop. In traditional loop, when I need to remove multiple elements, it crashes bcs I'm rearranging the array
            //So I had to cache first the elements needs to be removed in loop, after the loop ends, then I'd remove the elements with using the cache. But this erase-remove idiom completely fixed everything
            modifiers.erase(
                std::ranges::remove_if(modifiers,
                                       [&](const std::shared_ptr<ModType>& mod)
                                       {
                                           return mod->GetModifierName() == modifierName;
                                       }).begin(), modifiers.end());
        }

        //Get a specific type of modifier with same T 
        template <typename ModName>
        std::shared_ptr<ModName> GetModifier() const
        {
            for (const auto& mod : modifiers)
            {
                if (auto casted = std::dynamic_pointer_cast<ModName>(mod)) //attempts to cast each mod to a shared_ptr<T>
                    return casted;
            }
            return nullptr;
        }

        void ClearAllModifiers()
        {
            modifiers.clear();
        }

        //Check if a specific modifier exists
        [[nodiscard]] bool HasModifier(const std::string& modifierName) const
        {
            return std::ranges::any_of(modifiers, [&](const auto& mod)
            {
                return mod->GetModifierName() == modifierName;
            });
        }

        //Get number of active modifiers
        [[nodiscard]] size_t GetModifierCount() const
        {
            return modifiers.size();
        }

    private:
        std::vector<std::shared_ptr<ModType>> modifiers;
    };
}
