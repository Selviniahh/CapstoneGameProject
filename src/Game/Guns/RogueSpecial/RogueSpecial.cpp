#include "RogueSpecial.h"
#include <filesystem>
#include "../../../Engine/Core/Factory.h"
#include "../../../Engine/Managers/AssetManager.h"



ETG::RogueSpecial::RogueSpecial(const ETG::Vector2f& Position) : GunBase(Position,
0.35f,
200.f,
300.f,
0.f,
3.f,
300,
10,
2.0f,
3.5f,
35.f,
0.2f,
10.f)
{
    AnimationComp = CreateGameObjectAttached<RogueSpecialAnimComp>(this);
    SetShootSound(AssetManager::Resolve("Sounds/RogueSpecialShoot.ogg"));
    SetReloadSound(AssetManager::Resolve("Sounds/Reload.ogg"));

    // call the common initialization.
    RogueSpecial::Initialize();
}

void ETG::RogueSpecial::Initialize()
{
    GunBase::Initialize();
    
    ArrowComp->arrowOffset = {20.f, -6.f};

    // Set up the muzzle flash animation.
    MuzzleFlash->SetAnimation("Guns/RogueSpecial/MuzzleFlash/", "RS_muzzleflash_001", "png", 0.10f);
    MuzzleFlash->SetAttachmentOffset({37.f, -6.f});
    MuzzleFlash->Deactivate();

    // Reload VFX. MuzzleFlash defaults its origin to the centre of the whole stitched
    // sheet, which is meaningless for a multi-frame strip, so both of these pin it to
    // the pixel of their own frame that should land on the attachment point.
    ReloadFlash = CreateGameObjectAttached<class MuzzleFlash>(this);
    ReloadFlash->SetAnimation("Guns/RogueSpecial/Reload/MuzzleFlash/", "RogueSpecial_Reload_MuzzleFlash_001", "png", 0.06f);
    ReloadFlash->SetParent(this);
    ReloadFlash->SetAttachmentOffset({27.f, -8.f}); //barrel tip
    ReloadFlash->SetOrigin({15.f, 8.5f});                 //frames are drawn centred
    ReloadFlash->Deactivate();

    ReloadSmoke = CreateGameObjectAttached<class MuzzleFlash>(this);
    ReloadSmoke->SetAnimation("Guns/RogueSpecial/Reload/", "RogueSpecial_reload_smoke_001", "png", 0.12f);
    ReloadSmoke->SetParent(this);
    ReloadSmoke->SetAttachmentOffset({13,-10}); //under the open cylinder
    ReloadSmoke->SetOrigin({2.f, 11.f});                  //the wisp's root pixel
    ReloadSmoke->SetInheritParentRotation(false); //keep it rising when the gun aims left
    ReloadSmoke->Deactivate();
    

    // Load the projectile texture for RogueSpecial.
    ProjTexture = AssetManager::LoadTexture("Projectiles/RogueSpecial/Projectile_RogueSpecial.png");

}

void ETG::RogueSpecial::Update()
{
    GunBase::Update();

    ReloadFlash->Update();
    ReloadSmoke->Update();

    if (LastShot.ShotCount > 1)
    {
        // When firing a burst, make flash animation match bullet frequency
        MuzzleFlash->Animation.FrameInterval = ShotDelay / 2;
    }
    else
    {
        // Normal animation speed for single shots
        MuzzleFlash->Animation.FrameInterval = FireRate / 3;
    }
}

void ETG::RogueSpecial::Draw()
{
    GunBase::Draw();

    // IsVisible is cleared while the hero is dashing; the gun hides but its projectiles
    // keep drawing, and the reload VFX should follow the gun rather than the projectiles.
    if (!IsVisible) return;
    if (ReloadFlash->IsVisible) ReloadFlash->Draw();
    if (ReloadSmoke->IsVisible)
        ReloadSmoke->Draw();
}

void ETG::RogueSpecial::Reload()
{
    const bool wasReloading = IsReloading;

    GunBase::Reload();

    // GunBase::Reload bails out when the magazine is already full or a reload is already
    // running, so only fire the VFX when a reload genuinely began.
    if (!wasReloading && IsReloading)
    {
        ReloadFlash->Restart();
        ReloadSmoke->Restart();
    }
}

ETG::RogueSpecialAnimComp::RogueSpecialAnimComp()
{
    IsGameObjectUISpecified = true;
    RogueSpecialAnimComp::SetAnimations();
}

void ETG::RogueSpecialAnimComp::SetAnimations()
{
    //Idle Animation
    const Animation IdleAnim = {Animation::CreateSpriteSheet("Guns/RogueSpecial", "RogueSpecial_Idle", "png", 0.15f, true)};
    AddGunAnimationForState(GunStateEnum::Idle, Playback::Loop, IdleAnim, true, ETG::Vector2f{1,10});

    //Shoot animations
    Animation ShootAnim = {Animation::CreateSpriteSheet("Guns/RogueSpecial/Fire", "knav3_fire_001", "png", 0.15f)};
    AddGunAnimationForState(GunStateEnum::Shoot, Playback::Once, ShootAnim, true, ETG::Vector2f{1,10});

    //Reload Animation. 8 frames over the 2s reload time. The frames carry one spare row
    //above the gun so it can kick without shearing off its front sight, which is why the
    //origin is {1,11} here and {1,10} for the poses that have no such row.
    Animation ReloadAnim = {Animation::CreateSpriteSheet("Guns/RogueSpecial", "RogueSpecial_Reload", "png", 0.25f, true)};
    AddGunAnimationForState(GunStateEnum::Reload, Playback::Once, ReloadAnim, true, ETG::Vector2f{1,11});
}
