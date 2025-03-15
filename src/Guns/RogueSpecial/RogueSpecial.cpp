#include "RogueSpecial.h"
#include "../../Core/Factory.h"
#include <filesystem>

ETG::RogueSpecial::RogueSpecial(const sf::Vector2f& Position) : GunBase(Position, 0.35f, 200.f, 5000.f, 0.f)
{
    AnimationComp = CreateGameObjectAttached<RogueSpecialAnimComp>(this);

    // call the common initialization.
    RogueSpecial::Initialize();
}

void ETG::RogueSpecial::Initialize()
{
    OriginOffset = {1.f, 10.f};
    MuzzleFlashEachFrameSpeed = 0.10f;
    arrowOriginOffset = {-6.f, 0.f};
    arrowOffset = {20.f, -6.f};

    // Set up the muzzle flash animation.
    muzzleFlashAnim = Animation::CreateSpriteSheet("Guns/RogueSpecial/MuzzleFlash", "RS_muzzleflash_001", "png", MuzzleFlashEachFrameSpeed);
    if (muzzleFlashAnim.Texture)
    {
        muzzleFlashAnim.Origin = {
            static_cast<float>(muzzleFlashAnim.Texture->getSize().x / 2),
            static_cast<float>(muzzleFlashAnim.Texture->getSize().y / 2)
        };
    }
    muzzleFlashAnim.Active = false;
    MuzzleFlashOffset = {17.f, 0.f};

    // Load the projectile texture for RogueSpecial.
    // Fixed: Don't try to load before initializing ProjTexture
    const auto projPath = (std::filesystem::path(RESOURCE_PATH) / "Projectiles" / "RogueSpecial" / "Projectile_RogueSpecial.png").string();
    if (!ProjTexture->loadFromFile(projPath))
        throw std::runtime_error("Failed to load Projectile_RogueSpecial.png from path: " + projPath);

    GunBase::Initialize();
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
    AddGunAnimationForState(GunStateEnum::Idle, IdleAnim);

    //Shoot animations
    const Animation ShootAnim = {Animation::CreateSpriteSheet("Guns/RogueSpecial/Fire", "knav3_fire_001", "png", 0.15f)};
    AddGunAnimationForState(GunStateEnum::Shoot, ShootAnim);

    //Reload Animation
    const Animation ReloadAnim = {Animation::CreateSpriteSheet("Guns/RogueSpecial", "RogueSpecial_Reload", "png", 0.15f, true)};
    AddGunAnimationForState(GunStateEnum::Reload, ReloadAnim);
}
