#pragma once
#include <memory>
#include <vector>
#include "../../../Engine/Platform/Platform.h"
#include "GunBase.h"
#include <boost/describe.hpp>
#include "../../../Engine/Platform/Platform.h"
#include "../../../Engine/Platform/Platform.h"
#include "../../../Engine/Animation/Animation.h"
#include "../../../Engine/Core/GameObjectBase.h"
#include "../../../Engine/Core/Components/ArrowComp.h"
#include "../GunStates.h"
#include "../../../Engine/Core/Components/BaseAnimComp.h"
#include "../../../Engine/Core/Events/EventDelegate.h"
#include "../VFX/MuzzleFlash.h"
#include "../../Modifiers/ModifierManager.h"
#include "../../../Utils/Interface/IGunModifier.h"

namespace ETG
{
    class ReloadSlider;
    class ProjectileBase;
    struct QueuedBullet;
    enum class GunStateEnum;

    class GunBase : public GameObjectBase
    {
    public:
        GunBase(ETG::Vector2f Position, float fireRate, float shotSpeed, float range, float timerForVelocity, float depth, int ammoSize, int magazineSize, float reloadTime,
                float damage = 1.0f, float force = 1.0f, float spread = 0.0f);

        ~GunBase() override;
        void Initialize() override;
        void Update() override;
        void Draw() override;

    public:
        //When left click pressed from hero, this will be called. Based on the timer and fire rate, will be called to fire the bullets.
        //NOTE: This will handle every base gun should handle. They are: Check if shooting is possible, decrement magazine size, apply modifiers, broadcast event etc. NO SHOOTING LOGIC
        virtual void PrepareShooting(); //queue the bulletQueue

        //NOTE: This is the actual projectile firing logic. Override this without calling base in child to implement custom shooting logic
        virtual void EnqueueProjectiles(int shotCount, float EffectiveSpread);
        void RestartCurrentAnimStateAnimation();

        //NOTE: EnqueueProjectiles will add projectiles to the queue, this function will fire them in tick.
        void UpdateProjectiles(); //If projectile needs to be removed, remove and update

        virtual void Reload();
        void SetShootSound(const std::string& soundPath);
        void SetReloadSound(const std::string& soundPath);
        void FireBullet(float projectileAngle); //Fire an individual bullet
        [[nodiscard]] bool IsMagazineEmpty() const { return MagazineAmmo == 0; }; //Check if the magazine is empty

        //NOTE: only modifier we have so far. I didn't deem necessary to define this as smart pointer 
        ModifierManager<IGunModifier> modifierManager;

        std::vector<QueuedBullet> bulletQueue; //Queue of bullets waiting to be fired

        //NOTE: I am writing this cuz it's been 4th time I did same mistake. This delay is only related with active item's double shooting. Unless it's activated this won't be used
        //As best practice, I need to move this back to the Active item.  
        float ShotDelay = 0.1f;

        using GameObjectBase::Rotation; //Make Rotation public in Gunbase

        //State will not contain direction. It will be idle, shoot, reload etc. 
        GunStateEnum CurrentGunState{GunStateEnum::Idle};

        bool IsReloading{};

        //Base stat values (never modified, used as reference)
        float BaseFireRate;
        float BaseShotSpeed;;
        float BaseRange;
        float BaseReloadTime; //Time to reload
        float BaseDamage;
        float BaseForce;
        float BaseSpread;

        int BaseMaxAmmo{}; //Total ammo capacity 
        int BaseMagazineSize{}; //Bullets per magazine

        //Current effective stats (modified by items/ perks)
        float FireRate; //Time between shots (seconds)
        float ShotSpeed; //How fast bullets travel
        float Range; //How far bullets travel
        float ReloadTime; //Time to reload
        float Damage; //Damage per bullet
        float Force; //Knockback applied to enemies
        float Spread; //Bullet spread angle in degrees (0 = perfect accuracy)

        //Ammo stats
        int MaxAmmo{}; //Total ammo capacity 
        int MagazineSize{}; //Total bullets per magazine
        int MagazineAmmo{}; //Current magazine ammo count (this will be subtracted and reset)

        std::shared_ptr<ETG::Texture> ProjTexture;
        std::unique_ptr<ReloadSlider> ReloadSlider;
        EventDelegate<bool> OnAmmoRunOut;
        EventDelegate<bool> OnReloadInvoke;

    protected:
        float Timer; //Based on tick, this will increment 

        // Rotates an offset vector according to the gun's current rotation.
        std::vector<std::unique_ptr<ProjectileBase>> projectiles;
        std::unique_ptr<ArrowComp> ArrowComp;
        std::unique_ptr<MuzzleFlash> MuzzleFlash;

        //Gun needs to have custom Origin offset cuz, it needs to be attached to Hero's hand
        ETG::Vector2f OriginOffset;

        //Gun Animation
        std::unique_ptr<BaseAnimComp<GunStateEnum>> AnimationComp;

    private:
        //Sounds
        ETG::SoundBuffer ShootSoundBuffer;
        ETG::Sound ShootSound;

        ETG::SoundBuffer ReloadSoundBuffer;
        ETG::Sound ReloadSound;

        float ShootSoundVolume = 10;
        float ReloadSoundVolume = 10;

        BOOST_DESCRIBE_CLASS(GunBase, (GameObjectBase),
                             (CurrentGunState, MaxAmmo, MagazineSize, MagazineAmmo, ShotDelay, ReloadTime, IsReloading,
                                 FireRate, ShotSpeed, Range, Damage, Force, Spread),
                             (ProjTexture, OriginOffset),
                             ())
    };

    //A bullet for now only has time to fire and angle.
    struct QueuedBullet
    {
        float timeToFire;
        float angle;
    };
}
