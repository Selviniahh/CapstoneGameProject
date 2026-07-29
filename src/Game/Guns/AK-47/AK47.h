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

        float ReloadAnimInterval = 1.f; //Frame Count / Reload Time = Reload Time;

        //Shoot and Recoil are one-shots, so each plays its full 3 * interval before handing over,
        //and holding the trigger restarts the pair every 0.4s fire tick. For both to be seen in
        //full they have to fit inside that tick:
        //
        //    3*Shoot + 3*Recoil <= 0.4   ->   Shoot + Recoil <= 0.133
        //
        //0.05 + 0.08 spends 0.39s of it. Going over is not a crash - the next shot just cuts the
        //kick short, which is a fine look for a rifle - but go over deliberately, not by accident.
        float ShootAnimInterval = 0.05f;
        float RecoilAnimInterval = 0.08f;
        Vector2f AttachmentOrigin;
        BOOST_DESCRIBE_CLASS(AK47AnimComp, (BaseAnimComp), (AttachmentOrigin), (), ());
    };
}
