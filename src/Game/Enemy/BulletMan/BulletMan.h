#pragma once
#include "../EnemyBase.h"
#include "../../Characters/Hero.h"

namespace ETG
{
    class BulletManAnimComp;
    // Remove BulletManMoveComp forward declaration
    class Magnum;
    class ProjectileBase;

    class BulletMan : public EnemyBase
    {
    public:
        explicit BulletMan(const ETG::Vector2f& position);
        ~BulletMan() override;
        void Initialize() override;
        void Update() override;
        void Draw() override;
        
        void BulletManShoot(); //Shoot if timer is up and attackDistance
        void HandleHitForce(const ProjectileBase* projectile) override;

    protected:
        void UpdateAnimations();
        void UpdateHandAndGunPositions() const;
        void UpdateShooting();
        void UpdateVisibility() const;

    public:
        std::unique_ptr<BulletManAnimComp> AnimationComp;

        // Attack parameters...
        float attackCooldown = 2.0f;
        float attackCooldownTimer = 0.0f;

        std::unique_ptr<Magnum> Gun;

        

        BOOST_DESCRIBE_CLASS(BulletMan, (EnemyBase),
                             (attackCooldown, attackCooldownTimer),
                             (), ())
    };
}
