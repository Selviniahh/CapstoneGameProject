#include "RogueSpecial.h"
#include <filesystem>
#include "../../../Engine/Core/Factory.h"
#include "../../Modifiers/Gun/MultiShotModifier.h"
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
30.f,
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
    ArrowComp->arrowOffset = {20.f, -6.f};

    // Set up the muzzle flash animation.
    MuzzleFlash->Deactivate();
    MuzzleFlash->SetOffset({37.f, -6.f});

    // Load the projectile texture for RogueSpecial.
    ProjTexture = AssetManager::LoadTexture("Projectiles/RogueSpecial/Projectile_RogueSpecial.png");

    GunBase::Initialize();
}

void ETG::RogueSpecial::Update()
{
    GunBase::Update();

    if (modifierManager.GetModifier<MultiShotModifier>())
    {
        // When multishot is active, make flash animation match bullet frequency
        MuzzleFlash->Animation.FrameInterval = ShotDelay / 2;
    }
    else
    {
        // Normal animation speed for single shots
        MuzzleFlash->Animation.FrameInterval = FireRate / 3;
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
    AddGunAnimationForState(GunStateEnum::Idle, IdleAnim, true, ETG::Vector2f{1,10});

    //Shoot animations
    const Animation ShootAnim = {Animation::CreateSpriteSheet("Guns/RogueSpecial/Fire", "knav3_fire_001", "png", 0.15f)};
    AddGunAnimationForState(GunStateEnum::Shoot, ShootAnim, true, ETG::Vector2f{1,10});

    //Reload Animation
    const Animation ReloadAnim = {Animation::CreateSpriteSheet("Guns/RogueSpecial", "RogueSpecial_Reload", "png", 0.15f, true)};
    AddGunAnimationForState(GunStateEnum::Reload, ReloadAnim, true, ETG::Vector2f{1,10});
}
