#pragma once
#include "Platform/Platform.h"

#include "../../Animation/Animation.h"

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
