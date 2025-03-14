#pragma once
#include "../Core/GameObjectBase.h"

class ProjectileBase : public ETG::GameObjectBase
{
public:
    ProjectileBase() = default;
    ProjectileBase(const sf::Texture& texture,sf::Vector2f spawnPos, sf::Vector2f projVelocity, float maxProjectileRange, float rotation);

    void Initialize() override;
    void Update() override;
    void Draw() override;

    sf::Vector2f ProjVelocity;
    float MaxProjectileRang;

    BOOST_DESCRIBE_CLASS(ProjectileBase, (GameObjectBase), (ProjVelocity, MaxProjectileRang), (), ())
};
