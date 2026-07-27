#pragma once
#include <concepts>
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
        //NON-OWNING. A modifier is the item that granted it, and the item already lives in the world object list,
        //so the manager only borrows it. Whatever registers itself here is responsible for calling RemoveModifier
        //before it stops existing
        void AddModifier(ModType* modifier)
        {
            modifiers[std::type_index(typeid(*modifier))] = modifier;
        }

        //Identity of a modifier is its concrete type, so nothing has to be named or registered by hand.
        //Adding the same concrete type twice replaces the old one, which is what refreshing a timed effect should do
        template <typename T>
            requires std::derived_from<T, ModType>
        void RemoveModifier()
        {
            modifiers.erase(std::type_index(typeid(T)));
        }

        //O(1) lookup. Only matches the exact concrete type that was added
        template <typename T>
            requires std::derived_from<T, ModType>
        [[nodiscard]] T* GetModifier() const
        {
            const auto it = modifiers.find(std::type_index(typeid(T)));
            return it == modifiers.end() ? nullptr : static_cast<T*>(it->second);
        }

        template <typename T>
            requires std::derived_from<T, ModType>
        [[nodiscard]] bool HasModifier() const
        {
            return modifiers.contains(std::type_index(typeid(T)));
        }

        //Plain range-for support, so a caller that wants to give every modifier a turn just writes the loop:
        //    for (const auto& [type, modifier] : manager)
        //This is the counterpart to GetModifier - ask by type when you want one specific effect, loop when you
        //want whoever is listening. Order is the map's, so no modifier may depend on running before another
        [[nodiscard]] auto begin() const { return modifiers.begin(); }
        [[nodiscard]] auto end() const { return modifiers.end(); }

        void ClearAllModifiers()
        {
            modifiers.clear();
        }

        [[nodiscard]] size_t GetModifierCount() const
        {
            return modifiers.size();
        }

    private:
        std::unordered_map<std::type_index, ModType*> modifiers;
    };
}
