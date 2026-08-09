#include <filesystem>
#include "../../../Engine/Core/Factory.h"
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "../../Characters/Hero/Hero.h"
#include "SawedOff.h"
#include "../../../Utils/Math.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Managers/AssetManager.h"

ETG::SawedOff::SawedOff(const ETG::Vector2f& pos) : GunBase(pos,
                                                           1.8f, // FireRate
                                                           100.0f, // ShotSpeed
                                                           200.0f, // Range (infinite olmalı ama şimdilik 2000 vereceğim)
                                                           0.0f, // timerForVelocity
                                                           3.0f, // depth
                                                           165, // MaxAmmo
                                                           6, // MagazineSize
                                                           5.0f, // ReloadTime
                                                           5.5f, // Damage
                                                           50.f, // Force
                                                           1.f,
                                                           3.0f) // Spread (degree cinsinden)
{
    AnimationComp = CreateGameObjectAttached<SawedOffAnimComp>(this);
    SetShootSound(AssetManager::Resolve("Sounds/AK47Shoot.ogg"));
    SetReloadSound(AssetManager::Resolve("Sounds/AK47Reload.ogg"));

    CollisionComp = ETG::CreateGameObjectAttached<CollisionComponent>(this);
    CollisionComp->CollisionRadius = 1.f;
    CollisionComp->SetCollisionEnabled(true);

    SawedOff::Initialize();
}


void ETG::SawedOff::Initialize()
{
    GunBase::Initialize();
    // OriginOffset = {1.f, 5.f};
    ArrowComp->arrowOriginOffset = {-6.f, 0.f};
    ArrowComp->arrowOffset = {15.f, -4.f};
    CollisionComp->Initialize();

    ProjTexture = AssetManager::LoadTexture("Projectiles/bullet_variant_003.png");

    // Şimdilik silah hero ile collide olursa hero silahı alır. Bunu daha sonra hero içinde yapabiliriz.
    CollisionComp->OnCollisionEnter.AddListener([this](const CollisionEventData& eventData)
    {
        if (auto* hero = dynamic_cast<Hero*>(eventData.Other))
        {
            hero->EquipGun(this);
            CollisionComp->SetCollisionEnabled(false); // Equip sonrasında
            this->Owner = hero; // Silahın owner'ını hero yap. Projectile collision sırasında projectile owner'ını bilmemiz gerektiği için bu önemlidir.

        }
    });

    GunBase::Initialize();
}

void ETG::SawedOff::Update()
{
    MuzzleFlash->Deactivate();
    MuzzleFlash->IsVisible = false;
    CollisionComp->Update();
    ArrowComp->Update();
    GunBase::Update();
}

void ETG::SawedOff::Draw()
{
    GunBase::Draw();
    if (CollisionComp) CollisionComp->Visualize(*ETG::RenderContext::Window);
}

// Farklı bir işlem yapacağımız için base function'ı çağırmadan bu function'ı override etmemiz gerekir
void ETG::SawedOff::EnqueueProjectiles(const int shotCount, const float EffectiveSpread)
{
    // İlave bullet'ları delay ile queue'ya ekle
    for (int i = 0; i < shotCount; i++)
    {
        float projectileAngle = Rotation;
        float LastBulletSpread = 10;
        // Spread variation uygula
        if (EffectiveSpread > 0)
        {
            LastBulletSpread += Math::GenRandomNumber(-LastBulletSpreadAmount, LastBulletSpreadAmount);
        }
        
        // Bullet'ı queue'ya ekle
        bulletQueue.push_back({i * ShotDelay, projectileAngle});
        bulletQueue.push_back({i * ShotDelay, projectileAngle - 15});
        bulletQueue.push_back({i * ShotDelay, projectileAngle + 15});

        // Son bullet biraz spread ve delay ile ateşlenmeli
        bulletQueue.push_back({i * ShotDelay + Math::GenRandomNumber(LastBulletDelayMin,LastBulletDelayMax), projectileAngle + Math::GenRandomNumber(-LastBulletSpread, LastBulletSpread)});
    }

    // Ammo tükenmesini işle
    if (MagazineAmmo == 0)
    {
        OnAmmoRunOut.Broadcast(true);
    }
    
}

ETG::SawedOffAnimComp::SawedOffAnimComp()
{
    IsGameObjectUISpecified = true;
    SawedOffAnimComp::SetAnimations();
}

void ETG::SawedOffAnimComp::SetAnimations()
{
    BaseAnimComp<GunStateEnum>::SetAnimations();
    // Idle Animation ayarları
    const Animation IdleAnim = {Animation::CreateSpriteSheet("Guns/SawedOff", "sawed_off_shotgun_idle_001", "png", 0.15f, false)};
    IdleAnim.Origin = {1.f, 5.f};
    AddGunAnimationForState(GunStateEnum::Idle, Playback::Loop, IdleAnim, true, IdleAnim.Origin);

    // Shoot Animation ayarları
    Animation ShootAnim = {Animation::CreateSpriteSheet("Guns/SawedOff", "sawed_off_shotgun_fire_001", "png", ShootAnimInterval)};
    ShootAnim.Origin = {1.f, 5.f};
    AddGunAnimationForState(GunStateEnum::Shoot, Playback::Once, ShootAnim, true, ShootAnim.Origin);

    // Reload Animation ayarları
    Animation ReloadAnim = {Animation::CreateSpriteSheet("Guns/SawedOff", "sawed_off_shotgun_reload_001", "png", ReloadAnimInterval)};
    ReloadAnim.Origin = {5.f, 5.f};
    AddGunAnimationForState(GunStateEnum::Reload, Playback::Once, ReloadAnim, true, ReloadAnim.Origin);  
}
