#include "StatModifier.h"
#include <algorithm>
#include <cmath>

namespace ETG
{
    float StatModifier::Get() const
    {
        if (!IsDirty) return CachedValue;

        float flat = 0.f;
        float scale = 1.f;

        for (const auto& modifier : Modifiers)
        {
            switch (modifier.Op)
            {
            case StatOp::Flat:
                flat += modifier.Value;
                break;

                // 250’nin %10u:
                // 250 × 10 / 100 = 25
                
                // Yüzdeyi 0.10 şeklinde tutuyorsan:
                // float sonuc = 250.f * 0.10f;
                
                //Kısa formülü: float sonuc = 250.f * (1.f + 0.10f);
            case StatOp::Percent:
                scale *= (1.f + modifier.Value);
                break;
            }   
        }

        CachedValue = (BaseValue + flat) * scale;
        IsDirty = false;

        return CachedValue;
    }

    int StatModifier::GetInt() const
    {
        return static_cast<int>(std::lround(Get()));
    }

    void StatModifier::SetBase(const float base)
    {
        if (BaseValue == base) return;

        BaseValue = base;
        IsDirty = true;
    }

    void StatModifier::AddModifier(std::string source, const StatOp op, const float value)
    {
        //NOTE: Replace rather than append. An item's perk is a statement about what that item does right now, so
        //applying it twice has to mean the same thing as applying it once - otherwise a perk re-applied on a gun
        //switch, or after its percentage was dragged in the editor, would silently stack on top of its old self
        const auto existing = std::ranges::find_if(Modifiers, [&](const Stat& modifier)
        {
            return modifier.Source == source && modifier.Op == op;
        });

        if (existing != Modifiers.end())
        {
            if (existing->Value == value) return;

            existing->Value = value;
            IsDirty = true;
            return;
        }

        Modifiers.push_back(Stat{std::move(source), op, value});
        IsDirty = true;
    }

    bool StatModifier::RemoveModifiersFrom(const std::string& source)
    {
        const auto removed = std::ranges::remove_if(Modifiers, [&](const Stat& modifier)
        {
            return modifier.Source == source;
        });

        if (removed.begin() == Modifiers.end()) return false;

        Modifiers.erase(removed.begin(), Modifiers.end());
        IsDirty = true;

        return true;
    }

    bool StatModifier::HasModifierFrom(const std::string& source) const
    {
        return std::ranges::any_of(Modifiers, [&](const Stat& modifier)
        {
            return modifier.Source == source;
        });
    }

    void StatModifier::ClearModifiers()
    {
        if (Modifiers.empty()) return;

        Modifiers.clear();
        IsDirty = true;
    }
}
