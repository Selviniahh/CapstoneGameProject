#pragma once
#include "../Base/GunBase.h"

namespace ETG
{
    class CollisionComponent;
    class Magnum : public GunBase
    {
    public:
        explicit Magnum(const ETG::Vector2f& pos);
        ~Magnum() override = default;
        
        void Initialize() override;
        void Update() override;
        void Draw() override;
        void EnqueueProjectiles(int shotCount, float EffectiveSpread) override;
        
        std::unique_ptr<CollisionComponent> CollisionComp;

    protected:
        //Called from Magnum's constructor only - see GameObjectBase::BindEvents
        void BindEvents() override;

    public:
        BOOST_DESCRIBE_CLASS(Magnum, (GunBase), (), (), ());
    };

    class MagnumAnimComp : public BaseAnimComp<GunStateEnum>
    {
    public:
        MagnumAnimComp();
        void SetAnimations() override;
        float ShootAnimInterval = 0.1f;
        float ReloadAnimInterval = 3.f / 2.f; // Frame Count / Reload Time = Reload Time (negatif anim interval olamayacağı için önce büyük değeri bölmemiz gerekir)
        
        BOOST_DESCRIBE_CLASS(MagnumAnimComp, (BaseAnimComp), (), (), ())
    };
}
