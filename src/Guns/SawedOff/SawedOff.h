#pragma once
#include "../Base/GunBase.h"

namespace ETG
{
    class CollisionComponent;
    class SawedOff : public GunBase
    {
    public:
        explicit SawedOff(const ETG::Vector2f& pos);
        ~SawedOff() override = default;

        void Initialize() override;
        void Update() override;
        void Draw() override;
        void EnqueueProjectiles(int shotCount, float EffectiveSpread) override;

        float LastBulletSpreadAmount = 10;
        float LastBulletDelayMin = 0.05f;
        float LastBulletDelayMax = 0.4f;

        

        std::unique_ptr<CollisionComponent> CollisionComp;
        BOOST_DESCRIBE_CLASS(SawedOff, (GunBase), (LastBulletSpreadAmount), (),());
    };

    class SawedOffAnimComp : public BaseAnimComp<GunStateEnum>
    {
    public:
        SawedOffAnimComp();

        float ShootAnimInterval = 0.15f;
        float ReloadAnimInterval = 5 / 5.f; //Frame Count / Reload Time = Reload Time; 
        
        void SetAnimations() override;
        BOOST_DESCRIBE_CLASS(SawedOffAnimComp, (BaseAnimComp), (ShootAnimInterval, ReloadAnimInterval),(),());
    };
}
