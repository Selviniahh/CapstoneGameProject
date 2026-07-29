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
#include "../../Modifiers/Gun/IGunModifier.h"
#include "../../../Engine/Core/Stats/StatModifier.h"

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

        //Detaches everything `source` attached to any of this gun's stats. An item does not have to remember which
        //stats it touched, which is what makes "drop the item" the exact inverse of "pick the item up"
        void RemoveAllModifiersFrom(const std::string& source);

        virtual void Reload();
        void SetShootSound(const std::string& soundPath);
        void SetReloadSound(const std::string& soundPath);
        void FireBullet(float projectileAngle); //Fire an individual bullet
        [[nodiscard]] bool IsMagazineEmpty() const { return MagazineAmmo == 0; }; //Check if the magazine is empty

        ModifierManager<IGunModifier> modifierManager;

        //The shape the last shot ended up with, after the modifiers had their say. Lets a gun react to what it
        //actually fired - a burst needs a faster muzzle flash - without naming the modifier that caused it, so a
        //second modifier producing bursts is handled for free
        ShotParams LastShot{};

        std::vector<QueuedBullet> bulletQueue; //Queue of bullets waiting to be fired

        //NOTE: I am writing this cuz it's been 4th time I did same mistake. This delay is only related with active item's double shooting. Unless it's activated this won't be used
        //As best practice, I need to move this back to the Active item.  
        float ShotDelay = 0.1f;

        //How this particular gun sits in the hand, on top of the hand's own Hand::GunOffset.
        //Applied by Character::UpdateGuns, so it only affects the gun while it is being held -
        //a gun lying on the floor is unaffected.
        //NOTE: do NOT reach for Origin to nudge a gun. Origin is the pivot the gun rotates
        //around to aim, so shifting it swings the gun off-target by an amount that grows with
        //the aim angle. It is also rewritten from the animation every frame in
        //BaseAnimComp::Update, which is what makes the OriginOffset below a no-op.
        ETG::Vector2f HeldOffset{0.f, 0.f};

        using GameObjectBase::Rotation; //Make Rotation public in Gunbase

        //State will not contain direction. It will be idle, shoot, reload etc. 
        GunStateEnum CurrentGunState{GunStateEnum::Idle};

        bool IsReloading{};

        //Gun stats. Each one carries its own base value and whatever modifiers items have put on it.
        //
        //NOTE: There used to be two fields per stat - `BaseFireRate` alongside `FireRate` and so on - kept in sync by
        //hand in the constructor. The twin existed because an item that assigned to FireRate destroyed the only copy
        //of the unmodified number, so it needed somewhere to read it back from; that is also why PlatinumBullets
        //recomputed the whole stat from its base on every single Update. A Stat holds both halves, and an item never
        //assigns to it at all - it attaches a modifier under its own name and detaches it by that same name
        StatModifier FireRate; //Time between shots (seconds)
        StatModifier ShotSpeed; //How fast bullets travel
        StatModifier Range; //How far bullets travel
        StatModifier ReloadTime; //Time to reload
        StatModifier Damage; //Damage per bullet
        StatModifier Force; //Knockback applied to enemies
        StatModifier Spread; //Bullet spread angle in degrees (0 = perfect accuracy)

        StatModifier MagazineSize; //Bullets per magazine

        //NOTE: these two are counters, not stats, so they stay plain ints. MagazineAmmo is obviously one. MaxAmmo
        //reads like a capacity but ReloadSlider spends it (`MaxAmmo -= ...`), so it is really the reserve pool - the
        //name is lying and an item modifying it would be modifying the player's remaining bullets, not their capacity
        int MaxAmmo{};
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
                                 FireRate, ShotSpeed, Range, Damage, Force, Spread, HeldOffset),
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
