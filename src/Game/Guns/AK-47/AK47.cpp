#include "AK47.h"
#include <filesystem>
#include "../../../Engine/Core/Factory.h"
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "../../Characters/Hero/Hero.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Managers/AssetManager.h"

ETG::AK47::AK47(const ETG::Vector2f& pos) : GunBase(pos,
    0.6f,     // FireRate
    150.0f,     // ShotSpeed
    1000.0f,    // Range (should be infinite but I will just give 2000)
    0.0f,      // timerForVelocity
    -2.f,      // depth
    500,       // MaxAmmo
    30,        // MagazineSize
    2.0f,      // ReloadTime
    1.f,      // Damage
    20.0f,     // Force
    0.1f,     //force dur
    3.0f)      // Spread (in degrees)
{
    AnimationComp = CreateGameObjectAttached<AK47AnimComp>(this);
    SetShootSound(AssetManager::Resolve("Sounds/AK47Shoot.ogg"));
    SetReloadSound(AssetManager::Resolve("Sounds/AK47Reload.ogg"));
    
    CollisionComp = ETG::CreateGameObjectAttached<CollisionComponent>(this);
    CollisionComp->CollisionRadius = 1.f;
    CollisionComp->SetCollisionEnabled(true);

    // Call the common initialization
    AK47::Initialize();
}

void ETG::AK47::Initialize()
{
    ArrowComp->arrowOriginOffset = {-6.f, 0.f};
    ArrowComp->arrowOffset = {15.f, -2.f};
    // HeldOffset = {-1.f, 0.f}; //right: X -1, left: Character mirrors this to X +1

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

    CollisionComp->Initialize();
    
    // Load the projectile texture for AK-47
    ProjTexture = AssetManager::LoadTexture("Projectiles/bullet_variant_002.png");

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
}

void ETG::AK47::Draw()
{
    GunBase::Draw();
    if (CollisionComp) CollisionComp->Visualize(*ETG::RenderContext::Window);
}

bool ETG::AK47::WantsGripPinned() const
{
    //Pinned for exactly as long as the barrel is above the horizontal - the upper half of the circle on either
    //side, which is precisely where the hero shows his back and where an unpinned grip drops out from under his
    //sprite. Below the horizontal the grip is free and rides up and down with the barrel.
    //
    //NOTE: this used to be a latch, engaged here and released only when the gun changed hands. The release angle
    //being different from the engage angle is what suppressed the off hand across the whole lower right quadrant:
    //sweeping 0 -> 90 stayed pinned to the 0 degree reference, so the grip sat still while the original lifts it
    //with the barrel. There is nothing to remember - the pin is a property of where the gun points
    return GetRotation() > 180.f;
}

float ETG::AK47::PinnedGripRotation() const
{
    //The horizontal pose of the side the gun is held on. Picking the side's own horizontal is what makes the pin
    //engage with zero displacement: the barrel crosses that exact angle on its way up, so nothing jumps
    return IsHeldOnRightSide(GetRotation()) ? 0.f : 180.f;
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

    // Reload Animation
    Animation ReloadAnim = {Animation::CreateSpriteSheet("Guns/AK47", "ak47_reload_001", "png", ReloadAnimInterval, false)};
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
