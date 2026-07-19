#pragma once
#include "../../Engine/Platform/Platform.h"

#include "../../Engine/Animation/Animation.h"

namespace ETG
{
    // Interface for all animation components
    class IAnimationComponent
    {
    public:
        virtual ~IAnimationComponent() = default;
        [[nodiscard]] virtual ETG::IntRect GetCurrentTextureRect() const = 0;
        [[nodiscard]] const virtual Animation* GetAnimation() const = 0;
    };
}
