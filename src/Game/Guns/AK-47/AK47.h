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

        //3 shoot frames. IsAnimationFinished goes true on REACHING the last frame, so the shot
        //hands over to Recoil after (frames-1) * interval = 0.16s, not after all three frames.
        //Holding the trigger restarts this every 0.4s fire tick, so whatever is left of that
        //0.4s is all the time Recoil gets: at 0.15 only 0.10s remained and the kick was over
        //before you could see it. At 0.08 it gets 0.24s and plays out fully.
        float ShootAnimInterval = 0.16f;

        //3 frames, so the whole kick lasts 0.15s. Has to stay well under the 0.4s fire rate,
        //otherwise the next shot lands while the gun is still recoiling and it never settles.
        float RecoilAnimInterval = 0.2f;
        Vector2f AttachmentOrigin;
        BOOST_DESCRIBE_CLASS(AK47AnimComp, (BaseAnimComp), (AttachmentOrigin), (), ());
    };
}
