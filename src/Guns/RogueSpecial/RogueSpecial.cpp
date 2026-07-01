#include "RogueSpecial.h"
#include <filesystem>
#include "../../Core/Factory.h"
#include "../../Modifiers/Gun/MultiShotModifier.h"

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
    SetShootSound((std::filesystem::path(RESOURCE_PATH) / "Sounds" / "RogueSpecialShoot.ogg").generic_string());
    SetReloadSound((std::filesystem::path(RESOURCE_PATH) / "Sounds" / "Reload.ogg").generic_string());

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
    const auto projPath = (std::filesystem::path(RESOURCE_PATH) / "Projectiles" / "RogueSpecial" / "Projectile_RogueSpecial.png").string();
    if (!ProjTexture->loadFromFile(projPath))
        throw std::runtime_error("Failed to load Projectile_RogueSpecial.png from path: " + projPath);

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
