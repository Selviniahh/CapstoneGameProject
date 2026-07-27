#pragma once
#include <concepts>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace ETG
{
    //ModType is the marker interface of a modifier family (IGunModifier, IHeroModifier...). It has to be polymorphic since identity is taken from typeid
    template <typename ModType>
        requires std::is_polymorphic_v<ModType>
    class ModifierManager
    {
    public:
        //Identity of a modifier is its concrete type, NOT its name. GetModifierName is only for UI/debug so two modifiers sharing a name can no longer silently erase each other
        //Adding the same concrete type twice replaces the old one, which is what refreshing a timed effect should do
        void AddModifier(const std::shared_ptr<ModType>& modifier)
        {
            modifiers[std::type_index(typeid(*modifier))] = modifier;
        }

        template <typename T>
            requires std::derived_from<T, ModType>
        void RemoveModifier()
        {
            modifiers.erase(std::type_index(typeid(T)));
        }

        //O(1) lookup instead of walking the whole list with dynamic_pointer_cast. Only matches the exact concrete type that was added
        template <typename T>
            requires std::derived_from<T, ModType>
        [[nodiscard]] std::shared_ptr<T> GetModifier() const
        {
            const auto it = modifiers.find(std::type_index(typeid(T)));
            return it == modifiers.end() ? nullptr : std::static_pointer_cast<T>(it->second);
        }

        template <typename T>
            requires std::derived_from<T, ModType>
        [[nodiscard]] bool HasModifier() const
        {
            return modifiers.contains(std::type_index(typeid(T)));
        }

        void ClearAllModifiers()
        {
            modifiers.clear();
        }

        [[nodiscard]] size_t GetModifierCount() const
        {
            return modifiers.size();
        }

    private:
        std::unordered_map<std::type_index, std::shared_ptr<ModType>> modifiers;
    };
}
