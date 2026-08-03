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
                float damage = 1.0f, float force = 1.0f, float forceDuration = 1.0f, float spread = 0.0f);

        ~GunBase() override;
        void Initialize() override;
        void Update() override;
        void Draw() override;

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

        //How this particular gun artwork sits under stationary hands, on top of Hand::GunOffset. Author the value
        //for the right-held artwork; Character::UpdateGuns mirrors X when the gun changes sides. It only affects the
        //gun while it is held, so a gun lying on the floor is unaffected.
        //NOTE: do NOT reach for Origin to nudge a gun. Origin is the pivot the gun rotates
        //around to aim, so shifting it swings the gun off-target by an amount that grows with
        //the aim angle. It is also rewritten from the animation every frame in
        //BaseAnimComp::Update, which is what makes the OriginOffset below a no-op.
        ETG::Vector2f HeldOffset{0.f, 0.f};

        //Where each hand grips this gun, in the gun sheet's own pixels with (0,0) at the frame's
        //top-left corner - the numbers you read straight off the sprite in an image editor.
        //Character turns them into an offset from the gun's Origin, so the grips rotate and flip
        //with the gun for free, and a gun whose animation moves its origin still holds together.
        ETG::Vector2f RightHandAnchor{};
        ETG::Vector2f LeftHandAnchor{};

        //A hand is attached to the gun only for an anchor that has actually been measured. An unmeasured
        //off hand stays at the character's body instead; visibility belongs to the character.
        bool HasRightHandAnchor{false};
        bool HasLeftHandAnchor{false};

        //Which side of the body this gun is held on, and therefore which way its sprite is mirrored. A half
        //angle in degrees measured from straight right: the gun stays on the right hand while the aim is
        //within +-HandSwapAngle of straight right, and changes hands beyond it. 90 is the honest value - it
        //mirrors exactly when the barrel crosses vertical.
        //
        //NOTE: negative means the gun has no opinion, and the character falls back to its 8-way facing the way
        //every gun used to. That turns over at 67.5 degrees, because that is where the DownRight arc ends -
        //which leaves a 22.5 degree band where the gun is mirrored while still aiming to the right
        float HandSwapAngle{-1.f};

        [[nodiscard]] bool DecidesOwnHandSide() const { return HandSwapAngle >= 0.f; }

        //Whether the gun sits on the right hand for this aim. `aimAngle` is in [0,360), 0 straight right,
        //growing clockwise - the same convention DirectionUtils measures in. Only meaningful when the gun
        //decides its own side
        [[nodiscard]] bool IsHeldOnRightSide(float aimAngle) const;

        //Which hand the holder decided this gun is in this frame, written by Character::PublishHeldSideToGun
        //before anything reads it. A gun on the floor keeps whatever it last had, which nothing looks at.
        //
        //NOTE: the gun does not work this out for itself even though it has HandSwapAngle to do it with, because
        //a gun that leaves the angle negative has no opinion and the answer is then the *body's* facing - which
        //the gun cannot see. Being told is what makes every gun agree with its holder
        bool IsHeldOnRightHand{true};

        //<---------- Grip pinning ---------->
        //A two-handed gun looks broken when its forward grip swings out from under the holder while the barrel
        //points up: the off hand ends up in mid-air, off the sprite entirely. Pinning freezes that one pixel
        //against the body and lets the barrel swing around it - the gun still rotates, it just rotates about the
        //grip instead of about its own Origin.
        //
        //Set this and the two rules below do the whole job; a gun held in one hand leaves it false.
        //
        //NOTE: the pin belongs on the gun, not on the off hand. Pinning only the hand froze it in mid-air while
        //the gun carried on rotating out from under it - two locks that had to agree, and did not. The hands are
        //placed on the gun afterwards, so one lock on the gun is a lock on everything holding it
        bool PinsGripWhenAimingUp{false};

        //Whether the barrel points above the horizontal, either side. That is the half of the circle where the
        //holder is drawn from behind, and the only half where an unpinned grip leaves the sprite
        [[nodiscard]] bool IsBarrelAboveHorizontal() const { return GetRotation() > 180.f; }

        //Override only for a gun whose pin follows some other rule - a boss arm welded on permanently, say.
        //
        //NOTE: a pure function of the aim on purpose. This was a latch once, engaged when the barrel came up and
        //released only when the gun changed hands. Two different angles for the two halves of one rule is what
        //kept the grip frozen across a whole quadrant where it should have been free to ride up with the barrel
        [[nodiscard]] virtual bool WantsGripPinned() const
        {
            return PinsGripWhenAimingUp && IsBarrelAboveHorizontal();
        }

        //The angle the pinned grip is frozen at: the horizontal pose of whichever side the gun is held on. Picking
        //the side's own horizontal is what makes the pin engage with zero displacement - the barrel crosses that
        //exact angle on its way up, so nothing jumps at the moment the pin takes hold
        [[nodiscard]] virtual float PinnedGripRotation() const { return IsHeldOnRightHand ? 0.f : 180.f; }

        //Where the gun draws while it is held, in front of its holder's body and behind it. SpriteBatch sorts
        //greater depths first, so the larger of the two is the one behind. Both are seeded from the depth the gun
        //was constructed with, so a gun that says nothing about it keeps drawing exactly where it always did; a
        //gun that should disappear behind its holder's back gives HeldDepthBehindBody its own number.
        //
        //NOTE: one shared "behind" value on the character cannot work, because the guns do not agree on where
        //front is - RogueSpecial is authored at depth 3 and so is already behind a hero at -1, while the AK is at
        //-2 and in front of him. Behind is relative to a number only the gun knows
        float HeldDepthInFront{};
        float HeldDepthBehindBody{};

        float ForceDuration{1};

        using GameObjectBase::Rotation; //Make Rotation public in Gunbase
        using GameObjectBase::Depth; //Its holder rewrites this per frame from the two values above

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
                                 FireRate, ShotSpeed, Range, Damage, Force, ForceDuration, Spread, HeldOffset,
                                 RightHandAnchor, LeftHandAnchor, HasRightHandAnchor, HasLeftHandAnchor,
                                 HandSwapAngle, HeldDepthInFront, HeldDepthBehindBody),
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
