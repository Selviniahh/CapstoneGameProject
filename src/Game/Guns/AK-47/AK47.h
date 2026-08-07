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
        void Reload() override;

    public:

        //<---------- The reload performance ---------->
        //The forward hand comes off the handguard, reaches down to the magazine well beside the grip,
        //pulls up once and settles back - and the moment it arrives, the magazine drops out and falls.
        //
        //Every point below is measured on the RELOAD sheet's own 26x10 frame, not on the 27x7 idle frame
        //the hand anchors are authored against: while a reload plays, the reload artwork is what is on
        //screen, so its pixels are what the hands have to land on. The difference between the two is
        //carried by GunBase's hand gestures, so nothing here has to touch the authored anchors - and the
        //frames' different Origins are handled for free, because WorldPointOnGun measures every anchor
        //against whatever Origin the current frame wrote
        
        //Şarjörü değiştiren hareketli elin hedefidir. Bu el ön tutuşu bırakır, şarjör yuvasına gider ve şarjörü çeker.
        ETG::Vector2f ReloadMagWellPoint{11.f, 7.f}; //where the working hand stops, under the well
        
        //Kabzada/tetikte kalan diğer elin hedefidir. Bu el şarjöre gitmez; reload sırasında eğilen silahın kabzasını takip eder.
        ETG::Vector2f ReloadGripPoint{7.f, 7.f};     //the trigger hand's grip in the tilted reload pose
        ETG::Vector2f MagazineEjectPoint{11.f, 6.f}; //the pixel the magazine leaves the gun from

        //The three steps of the gesture, as fractions of ReloadTime. Reaching down runs until GrabEnd, the
        //up-and-down stroke until StrokeEnd, and the rest is the hand travelling back to the handguard
        float ReloadGrabEnd{0.3f};
        float ReloadStrokeEnd{0.75f};

        //How far up the stroke pulls, in gun pixels. This is the "yukarı çekip indirme" itself: a half sine
        //over the stroke, so the hand leaves the well and comes back to it without a corner at the top
        float ReloadStrokeHeight{3.f};

        //The shove the magazine leaves with, in pixels per second. Slightly backwards and up so it clears
        //the gun before gravity turns it over; X is mirrored with the gun so it never drops through the hero
        ETG::Vector2f MagazineEjectVelocity{-24.f, -34.f};

        BOOST_DESCRIBE_CLASS(AK47, (GunBase),
                             (ReloadMagWellPoint, ReloadGripPoint, MagazineEjectPoint, ReloadGrabEnd,
                                 ReloadStrokeEnd, ReloadStrokeHeight, MagazineEjectVelocity),
                             (), ())

    private:
        //Both hands' gestures for this frame, from where the reload has got to. Writes zero into them
        //whenever no reload is running, which is what puts the hands back on their authored anchors
        void UpdateReloadPerformance();

        //Throws the magazine away from the well. Called once per reload, at the moment the hand reaches it
        void EjectMagazine();
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
