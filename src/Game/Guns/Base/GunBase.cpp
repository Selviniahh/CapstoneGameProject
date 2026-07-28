#include "../../../Engine/Managers/Time.h"
#include <filesystem>
#include "GunBase.h"
#include <random>

#include "../../Characters/Hero/Hero.h"
#include "../../Projectile/ProjectileBase.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Managers/SpriteBatch.h"
#include "../../../Engine/Core/Factory.h"
#include "../../UI/UIObjects/ReloadSlider.h"
#include "../../../Utils/Math.h"
#include "../../Items/Passive/PlatinumBullets.h"
#include "../../../Engine/Managers/AssetManager.h"

namespace ETG
{
    GunBase::GunBase(const ETG::Vector2f Position,
                     const float fireRate,
                     const float shotSpeed,
                     const float range,
                     const float timerForVelocity,
                     const float depth,
                     const int maxAmmo,
                     const int magazineSize,
                     const float reloadTime,
                     const float damage,
                     const float force,
                     const float spread)
        //NOTE: a number assigned to a Stat sets its base, so these are the gun's unmodified values. The block that
        //used to copy every BaseX into its X down in the body is gone with the twins themselves
        : FireRate(fireRate), ShotSpeed(shotSpeed), Range(range), ReloadTime(reloadTime),
          Damage(damage), Force(force), Spread(spread),
          MagazineSize(magazineSize), MaxAmmo(maxAmmo), Timer(timerForVelocity)
    {
        // Initialize common position and textures
        this->Position = Position;
        this->Depth = depth;

        MagazineAmmo = MagazineSize.GetInt(); //Magazine needs to start with Magazine Ammo

        if (!Texture) Texture = std::make_shared<ETG::Texture>();
        if (!ProjTexture) ProjTexture = std::make_shared<ETG::Texture>();
        if (!Texture) Texture = std::make_shared<ETG::Texture>();
        if (!ArrowComp) ArrowComp = CreateGameObjectAttached<class ArrowComp>(this, AssetManager::Resolve("Projectiles/Arrow.png"));
        if (!MuzzleFlash) MuzzleFlash = CreateGameObjectAttached<class MuzzleFlash>(this, "Guns/RogueSpecial/MuzzleFlash/", "RS_muzzleflash_001", "png", 0.10f);
        ReloadSlider = ETG::CreateGameObjectAttached<class ReloadSlider>(this);

        GunBase::Initialize();
    }

    void GunBase::RemoveAllModifiersFrom(const std::string& source)
    {
 //        Döngünün her adımında stat, sıradaki nesnenin adresini alıyor. Mantıksal olarak şuna eşdeğer:
 //        FireRate.RemoveModifiersFrom(source);
 //        ShotSpeed.RemoveModifiersFrom(source);
 //        Range.RemoveModifiersFrom(source);
 //        ReloadTime.RemoveModifiersFrom(source);
 //        Damage.RemoveModifiersFrom(source);
 //        Force.RemoveModifiersFrom(source);
 //        Spread.RemoveModifiersFrom(source);
 //        MagazineSize.RemoveModifiersFrom(source);
        
        for (StatModifier* stat : {&FireRate, &ShotSpeed, &Range, &ReloadTime, &Damage, &Force, &Spread, &MagazineSize})
            stat->RemoveModifiersFrom(source);
    }

    GunBase::~GunBase()
    {
        for (auto& proj : projectiles)
            proj.reset();
    }

    void GunBase::Initialize()
    {
        Timer = FireRate + 1; //Set timer to be greater than fire rate so that we can shoot immediately

        //The origin manually needs to be given because when gun rotating, it has to rotate around the attachment point which is the handle point of the gun. 
        this->Origin += OriginOffset;
        ArrowComp->SetOrigin(ArrowComp->GetOrigin() + ArrowComp->arrowOriginOffset);

        // Muzzle flash animation should be set up by the derived class.
        MuzzleFlash->SetParent(this);

        ReloadSlider->LinkToGun(this);
    }

    void GunBase::Update()
    {
        GameObjectBase::Update();

        Timer += Time::FrameTick;

        //Fire all the bullets inside bulletQueue and remove them from the vector
        if (!bulletQueue.empty())
        {
            for (auto it = bulletQueue.begin(); it != bulletQueue.end();)
            {
                it->timeToFire -= Time::FrameTick;
                if (it->timeToFire <= 0)
                {
                    //Time to fie this bullet
                    FireBullet(it->angle);

                    it = bulletQueue.erase(it);
                }
                else
                    ++it;
            }
        }

        // After shooting, reset back to idle
        if (AnimationComp->CurrentState == GunStateEnum::Shoot &&
            AnimationComp->AnimManagerDict[AnimationComp->CurrentState].IsAnimationFinished())
        {
            CurrentGunState = GunStateEnum::Idle;
        }

        // Continue with the rest of the update logic
        ArrowComp->SetPosition(this->Position + Math::RotateVector(Rotation, Scale, ArrowComp->arrowOffset));
        ArrowComp->SetRotation(this->GetDrawProperties().Rotation);
        ArrowComp->Update();

        // Update gun animation.
        AnimationComp->Update(CurrentGunState, CurrentGunState);
        ComputeDrawProperties();

        // Update muzzle flash position and animation.
        MuzzleFlash->Update();

        // Update projectiles.
        UpdateProjectiles();

        ReloadSlider->Update();
    }

    void GunBase::Draw()
    {
        // Draw projectiles.
        for (const auto& proj : projectiles)
        {
            proj->Draw();
        }

        if (!IsVisible) return; //If dashing, this will be false and self gun shouldn't be drawn however projectiles should be. So we first draw projectiles then we draw self if visible 
        GameObjectBase::Draw();

        // Draw the gun.
        SpriteBatch::Draw(GetDrawProperties());

        // Draw the arrow representation.
        ArrowComp->Draw();

        // Draw the muzzle flash.
        if (MuzzleFlash->IsVisible) MuzzleFlash->Draw();
        ReloadSlider->Draw();
    }

    void GunBase::UpdateProjectiles()
    {
        for (auto it = projectiles.begin(); it != projectiles.end();)
        {
            (*it)->Update();
            if ((*it)->IsPendingDestroy())
            {
                UnregisterGameObject(it->get());

                //Because initialized projectile moved to this container with std::move: "projectiles.push_back(std::move(proj));", owner of the object is this container.
                //Simply removing the element from the vector will invoke
                //unique_ptr's destructor because unique_ptr requires 1 owner and since owner is gone, it'll automatically call destructor right away after this erase call.
                it = projectiles.erase(it); //After erase, set iterator to next iterator after the one removed
            }
            else
            {
                ++it;
            }
        }
    }

    void GunBase::PrepareShooting()
    {
        if (Timer >= FireRate)
        {
            //Reset firing timer
            Timer = 0;

            //The gun states what a plain shot looks like and lets the modifiers rewrite it. Kept in a local rather
            //than written back onto the gun, so nothing has to be restored when a timed effect runs out.
            //NOTE: no concrete modifier is named here on purpose. A new one that changes the shape of the shot is
            //written on its own and picked up by this loop without this function being touched again
            ShotParams shot{.ShotCount = 1, .Spread = Spread};
            for (const auto& [type, modifier] : modifierManager)
                modifier->ModifyShot(shot);

            LastShot = shot;

            //Consume ammo only once per shot group, however many bullets the modifiers asked for
            MagazineAmmo--;

            EnqueueProjectiles(shot.ShotCount, shot.Spread);
        }

        //Handle ammo depletion
        if (MagazineAmmo == 0)
        {
            OnAmmoRunOut.Broadcast(true);
        }
    }

    void GunBase::EnqueueProjectiles(const int shotCount, const float EffectiveSpread)
    {
        //Queue any additional bullets with delay (only useful if shotCount > 1) 
        for (int i = 0; i < shotCount; i++)
        {
            float projectileAngle = GameObjectBase::Rotation;

            //Apply spread variation
            if (EffectiveSpread > 0)
            {
                std::mt19937 engine(std::random_device{}());
                std::uniform_real_distribution<float> dist(-EffectiveSpread, EffectiveSpread);
                projectileAngle += dist(engine);
            }

            //Queue the bullet
            bulletQueue.push_back({i * ShotDelay, projectileAngle});
        }
    }

    void GunBase::FireBullet(float projectileAngle)
    {
        //Restart muzzle flash animation and shoot animation
        if (MuzzleFlash->IsVisible) MuzzleFlash->Restart();
        ShootSound.play();
        
        //Set animation state
        CurrentGunState = GunStateEnum::Shoot;
        RestartCurrentAnimStateAnimation();

        //Calculate spawn position
        const ETG::Vector2f spawnPos = ArrowComp->GetPosition();

        //Calculate velocity
        const float rad = Math::AngleToRadian(projectileAngle);
        const ETG::Vector2f direction = Math::RadianToDirection(rad);
        const ETG::Vector2f projVelocity = direction * ShotSpeed;

        //Spawn a projectile NOTE: I decided to instead spawn projectiles attached to fired gun. That's because in collision resolution, I need to know the owner gun of the projectile and from that learn whether it's a hero's or enemy's projectile
        //projectile.
        std::unique_ptr<ProjectileBase> proj = CreateGameObjectAttached<ProjectileBase>(this,*ProjTexture, spawnPos, projVelocity, Range, projectileAngle, Damage, Force);
        proj->Update();
        projectiles.push_back(std::move(proj));
    }

    void GunBase::Reload()
    {
        //IF already reloading or magazine is full do not invoke again
        if (IsReloading || MagazineAmmo == MagazineSize.GetInt()) return;
        
        CurrentGunState = GunStateEnum::Reload; //update animation
        RestartCurrentAnimStateAnimation();
        IsReloading = true;
        ReloadSound.play();
        OnAmmoRunOut.Broadcast(false); // Notify that we have ammo again
        OnReloadInvoke.Broadcast(true);
    }

    void GunBase::RestartCurrentAnimStateAnimation()
    {
        AnimationComp->AnimManagerDict[CurrentGunState].AnimationDict[CurrentGunState].Restart();
    }

    void GunBase::SetShootSound(const std::string& soundPath)
    {
        if (!ShootSoundBuffer.loadFromFile(soundPath))
            throw std::runtime_error("Failed to load gun sound");

        ShootSound.setBuffer(ShootSoundBuffer);
        ShootSound.setVolume(ShootSoundVolume);
    }

    void GunBase::SetReloadSound(const std::string& soundPath)
    {
        if (!ReloadSoundBuffer.loadFromFile(soundPath))
            throw std::runtime_error("Failed to load Reload sound");

        ReloadSound.setBuffer(ReloadSoundBuffer);
        ReloadSound.setVolume(ReloadSoundVolume);
    }
}
