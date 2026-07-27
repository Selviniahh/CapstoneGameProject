#pragma once
#include <string>

namespace ETG
{
    class IHeroModifier
    {
    public:
        virtual ~IHeroModifier() = default;
        [[nodiscard]] virtual std::string GetModifierName() const = 0;
    };
}
