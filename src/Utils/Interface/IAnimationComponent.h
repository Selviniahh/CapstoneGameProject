#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

namespace ETG
{
    // Interface for all animation components
    class IAnimationComponent
    {
    public:
        virtual ~IAnimationComponent() = default;
        [[nodiscard]] virtual sf::IntRect GetCurrentTextureRect() const = 0;
    };
}