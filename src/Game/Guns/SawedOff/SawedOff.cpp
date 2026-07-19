#include <filesystem>
#include "../../../Engine/Core/Factory.h"
#include "../../Modifiers/Gun/MultiShotModifier.h"
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "../../Characters/Hero.h"
#include "SawedOff.h"
#include "../../../Utils/Math.h"
#include "../../../Engine/Managers/AssetManager.h"

ETG::SawedOff::SawedOff(const ETG::Vector2f& pos) : GunBase(pos,
                                                           1.8f, // FireRate
                                                           100.0f, // ShotSpeed
                                                           200.0f, // Range (should be infinite but I will just give 2000)
                                                           0.0f, // timerForVelocity
                                                           3.0f, // depth
                                                           165, // MaxAmmo
                                                           6, // MagazineSize
                                                           5.0f, // ReloadTime
                                                           5.5f, // Damage
                                                           50.f, // Force
                                                           3.0f) // Spread (in degrees)
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

    const auto projPath = AssetManager::Resolve("Projectiles/bullet_variant_003.png");
    if (!ProjTexture->loadFromFile(projPath))
        throw std::runtime_error("Failed to load Projectile_SawedOff.png from path: " + projPath);

    //For now inside the gun if it collides with hero, hero will pick it up. We can do it instead inside hero later 
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
    if (CollisionComp) CollisionComp->Visualize(*GameState::GetInstance().GetRenderWindow());
}

//Because we will do something different, we need to override this function without calling base function
void ETG::SawedOff::EnqueueProjectiles(const int shotCount, const float EffectiveSpread)
{
    //Queue any additional bullets with delay
    for (int i = 0; i < shotCount; i++)
    {
        float projectileAngle = Rotation;
        float LastBulletSpread = 10;
        //Apply spread variation
        if (EffectiveSpread > 0)
        {
            LastBulletSpread += Math::GenRandomNumber(-LastBulletSpreadAmount, LastBulletSpreadAmount);
        }
        
        //Queue the bullet
        bulletQueue.push_back({i * ShotDelay, projectileAngle});
        bulletQueue.push_back({i * ShotDelay, projectileAngle - 15});
        bulletQueue.push_back({i * ShotDelay, projectileAngle + 15});

        //Last bullet should shoot with a bit of spread and delay
        bulletQueue.push_back({i * ShotDelay + Math::GenRandomNumber(LastBulletDelayMin,LastBulletDelayMax), projectileAngle + Math::GenRandomNumber(-LastBulletSpread, LastBulletSpread)});
    }

    //Handle ammo depletion
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
    // Idle Animation
    const Animation IdleAnim = {Animation::CreateSpriteSheet("Guns/SawedOff", "sawed_off_shotgun_idle_001", "png", 0.15f, false)};
    IdleAnim.Origin = {1.f, 5.f};
    AddGunAnimationForState(GunStateEnum::Idle, IdleAnim, true, IdleAnim.Origin);

    // Shoot Animations
    const Animation ShootAnim = {Animation::CreateSpriteSheet("Guns/SawedOff", "sawed_off_shotgun_fire_001", "png", ShootAnimInterval)};
    ShootAnim.Origin = {1.f, 5.f};
    AddGunAnimationForState(GunStateEnum::Shoot, ShootAnim, true, ShootAnim.Origin);

    // Reload Animation
    const Animation ReloadAnim = {Animation::CreateSpriteSheet("Guns/SawedOff", "sawed_off_shotgun_reload_001", "png", ReloadAnimInterval)};
    ReloadAnim.Origin = {5.f, 5.f};
    AddGunAnimationForState(GunStateEnum::Reload, ReloadAnim, true, ReloadAnim.Origin);  
}
