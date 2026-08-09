#include "Magnum.h"
#include "../../../Engine/Core/Factory.h"
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "../../Characters/Hero/Hero.h"
#include "../../Characters/Enemy/EnemyBase.h"
#include "../../../Utils/Math.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Managers/AssetManager.h"

ETG::Magnum::Magnum(const ETG::Vector2f& pos) : GunBase(pos,
                                                       1.8f, // FireRate
                                                       100.0f, // ShotSpeed
                                                       200.0f, // Range (infinite olmalı ama şimdilik 2000 vereceğim)
                                                       0.0f, // timerForVelocity
                                                       8.0f, // depth
                                                       165, // MaxAmmo
                                                       6, // MagazineSize
                                                       2.0f, // ReloadTime
                                                       5.5f, // Damage
                                                       4.f, // Force
                                                       1.f,
                                                       3.0f) // Spread (degree cinsinden)
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

    ProjTexture = AssetManager::LoadTexture("Projectiles/bullet_variant_003.png");

    // Şimdilik silah hero ile collide olursa hero silahı alır. Bunu daha sonra hero içinde yapabiliriz.
    CollisionComp->OnCollisionEnter.AddListener([this](const CollisionEventData& eventData)
    {
        // Önce other object'in Hero olup olmadığını kontrol et
        auto* hero = dynamic_cast<Hero*>(eventData.Other);

        // Ardından owner'ın EnemyBase OLMADIĞINI kontrol et (EnemyBase değilse cast nullptr döndürür)
        const auto* enemyOwner = dynamic_cast<EnemyBase*>(this->Owner);

        if (hero && !enemyOwner) // Hero null değilse VE owner enemy değilse
        {
            hero->EquipGun(this);
            CollisionComp->SetCollisionEnabled(false); // Equip sonrasında
            this->Owner = hero; // Silahın owner'ını hero yap. Projectile collision sırasında projectile owner'ını bilmemiz gerektiği için bu önemlidir.
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
    if (CollisionComp) CollisionComp->Visualize(*ETG::RenderContext::Window);
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

    // Idle Animation ayarları
    const Animation IdleAnim = {Animation::CreateSpriteSheet("Guns/Magnum", "magnum_idle_001", "png", 0.15f)};
    IdleAnim.Origin = {2.f, 12.f};
    AddGunAnimationForState(GunStateEnum::Idle, Playback::Loop, IdleAnim, true, IdleAnim.Origin);

    // Shoot Animation ayarları
    Animation ShootAnim = {Animation::CreateSpriteSheet("Guns/Magnum", "magnum_shoot_001", "png", ShootAnimInterval)};
    ShootAnim.Origin = {2.f, 12.f};
    AddGunAnimationForState(GunStateEnum::Shoot, Playback::Once, ShootAnim, true, ShootAnim.Origin);

    // Reload Animation ayarları
    Animation ReloadAnim = {Animation::CreateSpriteSheet("Guns/Magnum", "magnum_reload_001", "png", ReloadAnimInterval)};
    ReloadAnim.Origin = {2.f, 12.f};
    AddGunAnimationForState(GunStateEnum::Reload, Playback::Once, ReloadAnim, true, ReloadAnim.Origin);
}
