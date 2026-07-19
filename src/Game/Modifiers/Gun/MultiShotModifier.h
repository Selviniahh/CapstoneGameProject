#pragma once
#include <string>
#include "../../../Utils/Interface/IGunModifier.h"

namespace ETG
{
    //Modifier that changes number of bullets fired per shot
    class MultiShotModifier : public IGunModifier
    {
    public:
        explicit MultiShotModifier(const int shotCount, const float spread) : ShotCount(shotCount), Spread(spread) {}

        [[nodiscard]] int GetShotCount() const { return ShotCount; }
        [[nodiscard]] float GetSpread() const { return Spread; }
        [[nodiscard]] std::string GetModifierName() const override { return "MultiShot"; }

    private:
        int ShotCount;
        float Spread = 0.f;
    };    
}
