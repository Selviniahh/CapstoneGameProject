#include <filesystem>
#include "Magnum.h"
#include "../../Core/Factory.h"

ETG::Magnum::Magnum(const sf::Vector2f& position): GunBase(position, 0.35f, 200.f, 5000.f, 0.f)
{
    AnimationComp = CreateGameObjectAttached<MagnumAnimComp>(this);
    Magnum::Initialize();
}

void ETG::Magnum::Initialize()
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

ETG::MagnumAnimComp::MagnumAnimComp()
{
    IsGameObjectUISpecified = true;
    MagnumAnimComp::SetAnimations();
}

void ETG::MagnumAnimComp::SetAnimations()
{
    //Idle Animation
    const Animation IdleAnim = {Animation::CreateSpriteSheet("Guns/Magnum/Idle", "magnum_idle_001", "png", 0.15f, false)};
    AddGunAnimationForState(GunStateEnum::Idle, IdleAnim);

    //Shoot animations
    const Animation ShootAnim = {Animation::CreateSpriteSheet("Guns/Magnum/Fire", "magnum_enemy_fire_001", "png", 0.25f)};
    AddGunAnimationForState(GunStateEnum::Shoot, ShootAnim);

    //Reload Animation
    const Animation ReloadAnim = {Animation::CreateSpriteSheet("Guns/Magnum/Reload", "magnum_reload_001", "png", 0.15f, false)};
    AddGunAnimationForState(GunStateEnum::Reload, ReloadAnim);
}

bool ETG::MagnumAnimComp::IsAnimationFinished()
{
    auto it = AnimManagerDict.find(CurrentState);
    if (it != AnimManagerDict.end() && it->second.CurrentAnim)
    {
        return it->second.CurrentAnim->IsAnimationFinished();
    }
    return true;
}
