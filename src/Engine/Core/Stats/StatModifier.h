#pragma once
#include <string>
#include <vector>
#include <boost/describe.hpp>

// Bir şeyi modifier degilde stat yapan, sonsuza kadar sürmesi değil; sayısal bir değeri değiştirmesi.

namespace ETG
{
    //How a modifier folds into the value it is attached to.
    //op = Operation
    enum class StatOp
    {
        Flat, //Added to the base. "+2 max health"
        Percent //Scales the result. 0.20 is +20%, -0.20 is -20%
    };

    BOOST_DESCRIBE_ENUM(StatOp, Flat, Percent)

    //One item's or buff's claim on a stat. `Source` is what the removal is keyed on, so it has to identify the thing
    //that applied the modifier, not the effect: two different items that both raise fire rate need two different
    //sources, and one item that raises three stats can reuse the same source across all three
    struct Stat
    {
        std::string Source;
        StatOp Op{StatOp::Flat};
        float Value{0.f};
    };

    //A number a designer sets and items modify, without the two ever overwriting each other.
    //
    //NOTE: This replaces the pattern of a plain public float that items assigned to directly. That could not survive
    //a second item (last writer won) and could not survive an item being removed (the original value was gone), which
    //is why the guns had to carry a hand-maintained `BaseX` twin for every stat, and why PlatinumBullets recomputed
    //FireRate from BaseFireRate on every Update instead of applying its perk once.
    //
    //    final = (Base + sum of Flat) * product of (1 + Percent)
    //
    //Both halves are order independent, which is the property that makes removal exact: taking a modifier away gives
    //back precisely the value you would have had if it had never been added, no matter what else is attached. That is
    //also why Percent compounds rather than summing - summed percentages are not invertible one at a time
    class StatModifier
    {
    public:
        StatModifier() = default;

        //Implicit on purpose, so a stat can be written and initialised exactly like the float it replaced
        StatModifier(const float base) : BaseValue(base)
        {
        }

        //Assigning a bare number sets the BASE, it does not fight with the modifiers. This is the tuning path: a
        //constructor, the editor, `MoveComp->MaxSpeed = 100.f` in an enemy's setup. Items must go through AddModifier
        StatModifier& operator=(const float base)
        {
            SetBase(base);
            return *this;
        }

        //bir yerde float beklediğinde otomatik çalışır mesela burada
        // StatModifier MaxHealth = 10.f;
        // float health = MaxHealth;
        operator float() const { return Get(); }

        [[nodiscard]] float Get() const;

        //For the stats that are conceptually counts (magazine size, ammo capacity)
        [[nodiscard]] int GetInt() const;

        [[nodiscard]] float GetBase() const { return BaseValue; }
        void SetBase(float base);

        //Replaces any modifier the same source already had on this stat, so an item calling this every frame - or
        //re-applying itself after a value was tweaked in the editor - stacks w float()ith itself
        void AddModifier(std::string source, StatOp op, float value);

        //True if anything was actually removed
        bool RemoveModifiersFrom(const std::string& source);
        [[nodiscard]] bool HasModifierFrom(const std::string& source) const;

        void ClearModifiers();

        [[nodiscard]] const std::vector<Stat>& GetModifiers() const { return Modifiers; }

    private:
        float BaseValue{0.f};
        std::vector<Stat> Modifiers;

        //Get() runs per frame on stats like FireRate, so the fold is cached until something invalidates it
        mutable float CachedValue{0.f};
        mutable bool IsDirty{true};
    };
}
