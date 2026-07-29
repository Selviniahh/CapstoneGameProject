#pragma once
#include "../Base/GunBase.h"

namespace ETG
{
    class CollisionComponent;
    class AK47 : public GunBase
    {
    public:
        explicit AK47(const ETG::Vector2f& pos);
        ~AK47() override = default;

        void Initialize() override;
        void Update() override;
        void Draw() override;

    public:
        std::unique_ptr<CollisionComponent> CollisionComp;

        BOOST_DESCRIBE_CLASS(AK47, (GunBase), (), (),());
    };

    class AK47AnimComp : public BaseAnimComp<GunStateEnum>
    {
    public:
        AK47AnimComp();
        void SetAnimations() override;

        float ReloadAnimInterval = 2 / 2.f; //Frame Count / Reload Time = Reload Time; 
        Vector2f AttachmentOrigin;
        BOOST_DESCRIBE_CLASS(AK47AnimComp, (BaseAnimComp), (AttachmentOrigin), (), ());
    };
}
