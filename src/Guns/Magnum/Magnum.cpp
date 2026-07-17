#include "Magnum.h"
#include "../../Core/Factory.h"
#include "../../Modifiers/Gun/MultiShotModifier.h"
#include "../../Core/Components/CollisionComponent.h"
#include "../../Characters/Hero.h"
#include "../../Enemy/EnemyBase.h"
#include "../../Utils/Math.h"
#include "Managers/AssetManager.h"

ETG::Magnum::Magnum(const ETG::Vector2f& pos) : GunBase(pos,
                                                       1.8f, // FireRate
                                                       100.0f, // ShotSpeed
                                                       200.0f, // Range (should be infinite but I will just give 2000)
                                                       0.0f, // timerForVelocity
                                                       8.0f, // depth
                                                       165, // MaxAmmo
                                                       6, // MagazineSize
                                                       2.0f, // ReloadTime
                                                       5.5f, // Damage
                                                       4.f, // Force
                                                       3.0f) // Spread (in degrees)
{
    AnimationComp = CreateGameObjectAttached<MagnumAnimComp>(this);
    SetShootSound(AssetManager::Resolve("Sounds/AK47Shoot.ogg"));
    SetReloadSound(AssetManager::Resolve("Sounds/AK47Reload.ogg"));

    CollisionComp = ETG::CreateGameObjectAttached<CollisionComponent>(this);
    CollisionComp->CollisionRadius = 1.f;
    CollisionComp->SetCollisionEnabled(true);

    Magnum::Initialize();
}

void ETG::Magnum::Initialize()
{
    GunBase::Initialize();
    AnimationComp->Initialize();
    ArrowComp->arrowOffset = {20.f, -8.f};
    CollisionComp->Initialize();

    const auto projPath = AssetManager::Resolve("Projectiles/bullet_variant_003.png");
    if (!ProjTexture->loadFromFile(projPath))
        throw std::runtime_error("Failed to load Projectile_SawedOff.png from path: " + projPath);

    //For now inside the gun if it collides with hero, hero will pick it up. We can do it instead inside hero later
    CollisionComp->OnCollisionEnter.AddListener([this](const CollisionEventData& eventData)
    {
        // First check if the other object is a Hero
        auto* hero = dynamic_cast<Hero*>(eventData.Other);

        // Then check if owner is NOT an EnemyBase (the cast will return nullptr if it's not an EnemyBase)
        const auto* enemyOwner = dynamic_cast<EnemyBase*>(this->Owner);

        if (hero && !enemyOwner) // If hero is not null AND owner is not an enemy
        {
            hero->EquipGun(this);
            CollisionComp->SetCollisionEnabled(false); // After equip
            this->Owner = hero; //Set the owner of the gun to the hero This is important because during projectile collision, we need to know the owner of the projectile
        }
    });
}

void ETG::Magnum::Update()
{
    MuzzleFlash->Deactivate();
    MuzzleFlash->IsVisible = false;
    CollisionComp->Update();
    ArrowComp->Update();
    GunBase::Update();
}

void ETG::Magnum::Draw()
{
    GunBase::Draw();
    if (CollisionComp) CollisionComp->Visualize(*GameState::GetInstance().GetRenderWindow());
}

void ETG::Magnum::EnqueueProjectiles(int shotCount, float EffectiveSpread)
{
    GunBase::EnqueueProjectiles(shotCount, EffectiveSpread);
}

ETG::MagnumAnimComp::MagnumAnimComp()
{
    MagnumAnimComp::SetAnimations();
}

void ETG::MagnumAnimComp::SetAnimations()
{
    BaseAnimComp<GunStateEnum>::SetAnimations();

    // Idle Animation
    const Animation IdleAnim = {Animation::CreateSpriteSheet("Guns/Magnum", "magnum_idle_001", "png", 0.15f)};
    IdleAnim.Origin = {2.f, 12.f};
    AddGunAnimationForState(GunStateEnum::Idle, IdleAnim, true, IdleAnim.Origin);

    // Shoot Animations
    const Animation ShootAnim = {Animation::CreateSpriteSheet("Guns/Magnum", "magnum_shoot_001", "png", ShootAnimInterval)};
    ShootAnim.Origin = {2.f, 12.f};
    AddGunAnimationForState(GunStateEnum::Shoot, ShootAnim, true, ShootAnim.Origin);

    // Reload Animation
    const Animation ReloadAnim = {Animation::CreateSpriteSheet("Guns/Magnum", "magnum_reload_001", "png", ReloadAnimInterval)};
    ReloadAnim.Origin = {2.f, 12.f};
    AddGunAnimationForState(GunStateEnum::Reload, ReloadAnim, true, ReloadAnim.Origin);
}
