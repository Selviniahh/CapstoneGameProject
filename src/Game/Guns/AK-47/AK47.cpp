#include "AK47.h"
#include <algorithm>
#include <filesystem>
#include "../../../Engine/Core/Factory.h"
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "../../Characters/Hero/Hero.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Managers/AssetManager.h"
#include "../VFX/MagazineDrop.h"
#include "../../../Utils/Math.h"

ETG::AK47::AK47(const ETG::Vector2f& pos) : GunBase(pos,
    0.3f,     // FireRate
    200.0f,     // ShotSpeed
    1000.0f,    // Range (should be infinite but I will just give 2000)
    0.0f,      // timerForVelocity
    -2.f,      // depth
    500,       // MaxAmmo
    30,        // MagazineSize
    2.0f,      // ReloadTime
    0.5f,      // Damage
    20.0f,     // Force
    0.1f,     //force dur
    3.0f)      // Spread (in degrees)
{
    AnimationComp = CreateGameObjectAttached<AK47AnimComp>(this);
    SetShootSound(AssetManager::Resolve("Sounds/AK47Shoot.ogg"));
    SetReloadSound(AssetManager::Resolve("Sounds/AK47Reload.ogg"));
    
    CollisionComp = ETG::CreateGameObjectAttached<CollisionComponent>(this);

    // Call the common initialization
    AK47::Initialize();
}

void ETG::AK47::Initialize()
{
    ArrowComp->arrowOriginOffset = {-6.f, 0.f};
    ArrowComp->arrowOffset = {15.f, -2.f};

    //Read off the 27x7 idle frame with (0,0) at its top-left: the trigger hand on the grip, the other one
    //forward on the magazine. A rifle is held with both, so both anchors are live
    //TODO: Ayni silahi düşmanda kullanmasını isteyeceğim. O yüzden eğer owner heroysa bunu yap eğer owner'ım bulletman veya diğer düşmansa böyle yap diye devam edeceğiz
    RightHandAnchor = {17.f, 3.f};
    LeftHandAnchor = {7.f, 5.f};
    HasRightHandAnchor = true;
    HasLeftHandAnchor = true;

    //Stays in the right hand until the barrel is straight down, instead of turning over at the 67.5 degrees the
    //body's 8-way facing does. A rifle is the worst case for that band: it is long enough that being mirrored
    //while still aiming to the right swings the whole barrel across the hero
    HandSwapAngle = 90.f;

    //Behind the hero (-1) once he turns his back, in front of him otherwise. Sits behind the hands' own back
    //depth as well, so the rifle does not cover the fingers gripping it
    HeldDepthBehindBody = 1.f;

    //A rifle is held with both hands, so the forward grip is welded to the body whenever the barrel comes up
    //above the horizontal. GunBase's default rules cover the rest; there is nothing to override
    PinsGripWhenAimingUp = true;

    CollisionComp->Initialize();

    //GunBase owns the object; all this gun has to say is which magazine falls out of it
    Magazine->SetSprite("Guns/AK47/ak47_clip_001.png");

    // Load the projectile texture for AK-47
    ProjTexture = AssetManager::LoadTexture("Projectiles/AK-47/Projectile_AK-47.png");

    CollisionComp->OnCollisionEnter.AddListener([this](const CollisionEventData& eventData)
    {
        if (auto* hero = dynamic_cast<Hero*>(eventData.Other))
        {
            hero->EquipGun(this);
            CollisionComp->SetCollisionEnabled(false); //After equip
            this->Owner = hero; //Set the owner of the gun to the hero This is important because during projectile collision, we need to know the owner of the projectile
        }
    });

    GunBase::Initialize();
}

void ETG::AK47::Update()
{
    MuzzleFlash->Deactivate();
    MuzzleFlash->IsVisible = false;
    CollisionComp->Update();
    ArrowComp->Update();
    GunBase::Update();

    //After GunBase::Update, never before: it advances the reload clock the performance is timed off, and it
    //is where the animation writes the Origin every anchor below is measured against. Still early enough for
    //this frame's hands, because the holder places those after every gun has ticked (Character::UpdateGuns)
    UpdateReloadPerformance();
}

void ETG::AK47::Draw()
{
    GunBase::Draw();
}

void ETG::AK47::Reload()
{
    GunBase::Reload();
}

//<---------- The reload performance ---------->
void ETG::AK47::UpdateReloadPerformance()
{
    //No reload, no performance: a zero gesture is what puts both hands back on their authored anchors
    if (!IsReloading)
    {
        RightHandGesture = {};
        LeftHandGesture = {};
        return;
    }

    const float progress = ReloadProgress();

    //Where the working hand has to get to, expressed as a displacement from its own anchor. Starting and
    //ending at zero is what keeps the hand continuous with the idle pose at both ends of the reload
    //magazine well = şarjör yüvası           Reload başladığında çalışan el: RightHandAnchor'dan ReloadMagWellPoint'e gidecek
    const ETG::Vector2f toWell = ReloadMagWellPoint - RightHandAnchor;

    //How far the reload has taken the hands over from their idle grips: 0 at both ends, 1 across the stroke.
    //NOTE: every beat below divides by its own length, and all three lengths are editor-tweakable. The floors
    //are there so that a beat somebody has squeezed to nothing costs a hard cut, not a NaN sent to a hand
    float engagement;
    ETG::Vector2f gesture;

    if (progress < ReloadGrabEnd)
    {
        //Reaching down. The hand comes off the handguard and travels to the magazine well
        engagement = progress / std::max(ReloadGrabEnd, 0.001f);
        gesture = toWell * engagement;
    }
    else if (progress < ReloadStrokeEnd)
    {
        //The stroke itself - the pull up and back down. A half sine leaves zero and returns to it, so the
        //hand has no corner at the top of the pull the way a triangle wave would
        const float strokeTime = (progress - ReloadGrabEnd) / std::max(ReloadStrokeEnd - ReloadGrabEnd, 0.001f);

        engagement = 1.f;
        gesture = toWell + ETG::Vector2f{0.f, -ReloadStrokeHeight * Math::BellCurve(strokeTime)};
    }
    else
    {
        //Back onto the handguard, landing exactly on the anchor as the reload ends
        engagement = (1.f - progress) / std::max(1.f - ReloadStrokeEnd, 0.001f);
        gesture = toWell * engagement;
    }

    RightHandGesture = gesture;

    //The trigger hand never leaves the grip. All it does is follow the grip's own move into the tilted
    //reload pose, faded on the same weight so it arrives and leaves with the hand doing the work
    LeftHandGesture = (ReloadGripPoint - LeftHandAnchor) * engagement;

    //The magazine leaves the moment the hand arrives at the well, which is the beat the whole gesture reads
    //from: the hand pulls, and what it pulled out falls away
    if (!MagazineEjected && progress >= ReloadGrabEnd) EjectMagazine();
}

void ETG::AK47::EjectMagazine()
{
    MagazineEjected = true;

    //Mirrored with the gun, so the magazine is always thrown out behind the rifle rather than through the
    //hero when he aims left
    ETG::Vector2f velocity = MagazineEjectVelocity;
    if (!IsHeldOnRightHand) velocity.x = -velocity.x;

    //Dropped at the gun's own depth: it starts off exactly where the well is, so it has to sort with the
    //rifle rather than pop in front of or behind it on the frame it appears
    Magazine->Drop(WorldPointOnGun(MagazineEjectPoint), Rotation, velocity, Depth);
}

ETG::AK47AnimComp::AK47AnimComp()
{
    IsGameObjectUISpecified = true;
    AK47AnimComp::SetAnimations();
}

void ETG::AK47AnimComp::SetAnimations()
{
    BaseAnimComp<GunStateEnum>::SetAnimations();

    // Idle Animation
    AttachmentOrigin = {0.f, 0.f};
    
    // interval = kare başına süre, animasyonun toplam süresi değil
    const Animation IdleAnim = {Animation::CreateSpriteSheet("Guns/AK47", "AK47_Single001", "png", 0.15f, false)};
    AddGunAnimationForState(GunStateEnum::Idle, Playback::Loop, IdleAnim);

    // Shoot animations
    Animation ShootAnim = {Animation::CreateSpriteSheet("Guns/AK47", "ak47_shoot_001", "png", ShootAnimInterval)};
    AddGunAnimationForState(GunStateEnum::Shoot, Playback::Once, ShootAnim);

    // Reload Animation. One frame on purpose: the magazine-out pose, held for the whole reload.
    //NOTE: the two-frame ak47_reload_001 sheet bakes the dropped magazine into its second frame, which would
    //stand still under the gun next to the MagazineDrop actually falling - two magazines, one of them frozen.
    //ak47_reload_pose_001 is that sheet's first frame on its own, so the artwork shows the gun with its
    //magazine gone and the falling one is the object AK47::EjectMagazine throws
    Animation ReloadAnim = {Animation::CreateSpriteSheet("Guns/AK47", "ak47_reload_pose_001", "png", ReloadAnimInterval, false)};
    AddGunAnimationForState(GunStateEnum::Reload, Playback::Once, ReloadAnim);
    
    // Recoil Animation. Plays once after the shoot animation, then GunBase drops back to Idle.
    Animation RecoilAnim = {Animation::CreateSpriteSheet("Guns/AK47", "ak47_shoot_recoil_001", "png", RecoilAnimInterval, false)};
    AddGunAnimationForState(GunStateEnum::Recoil, Playback::Once, RecoilAnim);
}
//
// │        │ kare │ interval │  teslim süresi   │
// ├────────┼──────┼──────────┼──────────────────┤
// │ Shoot  │ 3    │ 0.08     │ 2 × 0.08 = 0.16s │
// ├────────┼──────┼──────────┼──────────────────┤
// │ Recoil │ 3    │ 0.10     │ 2 × 0.10 = 0.20s │
// ├────────┼──────┼──────────┼──────────────────┤
// │        │      │          │ toplam 0.36s     │
// └────────┴──────┴──────────┴──────────────────┘

//bizim zaten 0.4 saniye hakkimiz vardi 
