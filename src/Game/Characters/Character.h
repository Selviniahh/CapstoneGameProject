#pragma once
#include <memory>
#include <vector>
#include "../../Engine/Core/GameObjectBase.h"
#include "../../Engine/Core/Direction.h"

namespace ETG
{
    class ActiveItemBase;
    class BaseHealthComp;
    class BaseMoveComp;
    class CollisionComponent;
    class GunBase;
    class Hand;
    class PassiveItemBase;

    //Everything that is true of anyone walking around the dungeon shooting things: a body that moves and can be hit,
    //a hand holding one gun out of an inventory, and a bag of items it can trigger.
    //
    //NOTE: Hero and EnemyBase used to be two independent GameObjectBase children that happened to grow the same
    //members - a collision comp, a health comp, a move comp, a hand, a gun, facing - and then diverged in the
    //details (the hero's gun lived in a list, the enemy's in a unique_ptr; the hero could hold active items, the
    //enemy could not). What actually differs between them is who decides what to do next: a player's mouse, or an
    //AI. That decision is the derived classes' whole job; the parts below belong to both.
    class Character : public GameObjectBase
    {
    protected:
        //Abstract on purpose: a bare Character has no way to answer the capability questions below, so it is never
        //spawned. Components are built by the derived classes, which know their own radii, health and speeds
        Character();

    public:
        ~Character() override;

        //<---------- Capabilities ---------->
        //Pure virtual because the two branches answer them from completely different machinery - Hero from its
        //hierarchical state machine, EnemyBase from a flat flag set - while everything that asks (items, guns, the
        //shared helpers below) only ever needs the answer
        [[nodiscard]] virtual bool IsAlive() const = 0;
        [[nodiscard]] virtual bool CanMove() const = 0;
        [[nodiscard]] virtual bool CanShoot() const = 0;
        [[nodiscard]] virtual bool CanFlipAnims() const = 0;
        [[nodiscard]] virtual bool CanTakeDamage() const = 0;
        [[nodiscard]] virtual bool CanSwitchGuns() const = 0;
        [[nodiscard]] virtual bool CanUseActiveItems() const = 0;

        //<---------- Per-frame work shared by every character ---------->
        //THE HELD-GUN RIG. Everything that decides where a held gun and the two hands on it end up, split one
        //decision per function. Every character that carries a gun runs the identical set - hero, BulletMan, and
        //whatever boss comes next - so a fix here is a fix for all of them, and a new shooter needs no new
        //geometry at all. The per-gun knobs it reads are the ones documented in GunBase; the whole rig, its
        //ordering and the reasoning behind each step is written up in docs/HeldGunRig.tr.md.
        //
        //Only three of these are called from outside, in this order, and the order is not a preference: each step
        //reads values the previous one wrote.
        //
        //   1. UpdateHoldPoint()             the joint the gun hangs from
        //   2. UpdateGuns()                  the gun onto the joint, then the hands onto the gun
        //   3. UpdateHandAndGunVisibility()  whether any of it is drawn at all
        //
        //Left out of Update() deliberately: the two branches run these against their own state machinery, and
        //when in their tick they run is the part they do not agree on
        void UpdateHoldPoint();
        void UpdateGuns();
        void UpdateHandAndGunVisibility() const;

        //Which hand the current gun is in, and therefore which way its sprite is mirrored. THE single decider -
        //every step below asks this and none of them decides it again. The gun answers for itself when it names a
        //GunBase::HandSwapAngle (a revolver changing hands exactly at vertical); otherwise the body's 8-way facing
        //answers, which is what every gun did before the knob existed.
        //
        //NOTE: this being one function is the whole point. The hold point, the mirror and the hands used to work
        //it out separately, so a gun with its own swap angle had its sprite turn over 22.5 degrees away from the
        //hand holding it
        [[nodiscard]] bool IsGunOnRightSide() const;

        //<---------- Rendering ---------->
        //Characters draw through the grayscale sprite program by default; this turns it off (or back
        //on) for a single character. How strong "on" is, is global: GraphicsDevice::SetGrayscaleAmount.
        void SetGrayscaleEnabled(bool enabled);
        [[nodiscard]] bool IsGrayscaleEnabled() const { return Effect == ShaderEffect::Grayscale; }

    protected:
        //<---------- The rig, step by step ---------->
        //Protected rather than private: a boss with a stranger rig can reuse the steps it likes and replace the
        //rest, instead of copying the geometry

        //Tells the gun which hand it ended up in, so the gun's own rules (PinnedGripRotation) can read the
        //decision instead of working it out a second time from an angle it would have to interpret itself
        void PublishHeldSideToGun() const;

        //The gun onto the hold point, aimed down AimAngle. Nothing about hands or mirroring here
        void PlaceHeldGun() const;

        //Mirrors the gun vertically while it is held on the left, so its sprite is never upside down. Skipped
        //entirely while the character may not flip its animations (mid-dash, dead)
        void MirrorHeldGun() const;

        //Puts the gun in front of the body or behind it, from the gun's own two depths. Runs before the gun ticks,
        //because a gun bakes its depth into its draw properties inside its own Update
        void UpdateHeldGunDepth() const;

        //Honours a gun that asked to turn about its left-hand grip instead of about its own Origin, by sliding the
        //whole gun until that grip lands back on its pinned point. Runs after the rotation and the mirror are
        //settled, because it reads both, and before the hands are placed - so the hands ride the pin for free
        void ApplyGripPin() const;

        //Every equipped gun ticks, not only the one in hand: the holstered ones still have projectiles in flight
        void TickEquippedGuns() const;

        //Both hands onto the pixels the current gun named for them. Called from UpdateGuns AFTER the guns have
        //ticked, because it reads the Origin their animations just wrote
        void UpdateHands() const;

        //Which side of the body the hands draw on. Same rule the gun's depth follows, for the same reason
        void UpdateHandDepths() const;

        //Places one hand, on the gun if the gun named a grip for it and against the body if it did not. Both
        //hands go through this: they differ only in which anchor and which resting offset they are given.
        //`gripAnchor` already carries whatever the gun is acting out this frame (GunBase's hand gestures),
        //because from here a hand on a grip and a hand halfway through a reload are the same job
        void PlaceHand(class Hand& hand, bool gunNamesGrip, const ETG::Vector2f& gripAnchor,
                       const ETG::Vector2f& bodyRestPosition) const;

        //The gun's HeldOffset with its X mirrored when the gun is held on the left, so a value authored against
        //the right-held artwork keeps meaning the same thing on the other side. Read by the gun's placement and
        //by the hands, which is exactly why it is a function and not two copies of the ternary
        [[nodiscard]] ETG::Vector2f MirroredHeldOffset() const;

        //A point offset from the body, in world space. The body's own mirror is deliberately NOT fed into the
        //rotation: facing is already baked into whichever offset the caller picked, and passing Scale.x = -1 in
        //as well would mirror it a second time
        [[nodiscard]] ETG::Vector2f BodyRestPosition(const ETG::Vector2f& offset) const;

    public:

        //Fires the held active item if the character is currently allowed to. Reads no input: the hero calls this
        //from its Space binding, an enemy calls it whenever its AI decides to
        void UseActiveItem() const;

        //<---------- Gun inventory ---------->
        //The guns are non-owning: each character owns its starting weapon as a concrete type (Hero::RogueSpecial,
        //BulletMan::Gun) and hands it to EquipGun, and world pickups are owned by the scene
        void EquipGun(GunBase* newGun);
        void SwitchGun(int offset);
        void SwitchToNextGun();
        void SwitchToPreviousGun();
        [[nodiscard]] GunBase* GetCurrentHoldingGun() const { return CurrentGun; }

        //Only the gun in hand is drawn; the rest still tick, because their projectiles are in flight
        void ShowOnlyCurrentGun() const;

        //<---------- Items ---------->
        //NOTE: pickup used to be split between the item (which pushed itself onto the hero's list) and the hero
        //(whose collision listener set CurrActiveItem). Two halves of one job, and only the hero half checked
        //nothing. Items call these instead, which is also what lets an enemy pick the same item up
        void PickUpActiveItem(ActiveItemBase* item);
        void PickUpPassiveItem(PassiveItemBase* item);

    public:
        std::unique_ptr<CollisionComponent> CollisionComp;
        std::unique_ptr<BaseHealthComp> HealthComp;

        //Held as the base type because Hero and EnemyBase drive movement in incompatible ways (a dash with a
        //cooldown vs. chase-the-hero AI). Each side reaches its own type through the typed GetMoveComp() it declares
        std::unique_ptr<BaseMoveComp> MoveComp;

        //Both hands are their own sprite now - the art no longer bakes one into the body. Which pixel of the gun
        //each one grips is the gun's business (GunBase::RightHandAnchor / LeftHandAnchor); when a gun does not name
        //a second grip, OffHand remains visible at the opposite side of the body
        std::unique_ptr<class Hand> Hand;
        std::unique_ptr<class Hand> OffHand;

        //Where the character is facing, and where it is aiming. The hero gets both from the mouse, an enemy from
        //the direction of its target - which is why the angle is not called MouseAngle any more
        Direction CurrentDir{Direction::Right};
        float AimAngle{};
        bool IsShooting{};

        std::vector<GunBase*> EquippedGuns;
        GunBase* CurrentGun{nullptr};
        int CurrentGunIndex{0};

        //The active item Space (or an AI decision) triggers, plus everything picked up so far. The UI reads these
        //to draw the equipped slots
        ActiveItemBase* CurrActiveItem{nullptr};
        std::vector<ActiveItemBase*> EquippedActiveItems;
        std::vector<PassiveItemBase*> EquippedPassiveItems;

        //Where the hand sits relative to the body, per facing. Public so each character keeps its own art's numbers
        ETG::Vector2f HandOffsetRight{8.f, 5.f};
        ETG::Vector2f HandOffsetLeft{-8.f, 5.f};

        //Where the hands sit in the draw order, in front of the body and behind it. SpriteBatch sorts greater
        //depths first, so the larger of the two is the one that ends up behind: in the up-facing animations the
        //character is drawn from behind and its hands are then on the far side of it.
        //
        //NOTE: the hands do not carry a depth of their own any more. It has to change with facing, and a value
        //written once in Hand's constructor cannot
        float HandDepthInFront{-3.f};
        float HandDepthBehindBody{0.f};

        //Where the body holds the gun up, in world space. The hands used to be here and the gun hung off them;
        //now the gun hangs off this point and the hands hang off the gun, which is the only ordering that lets a
        //gun place its own grips. Nothing draws here - it is a joint, not a sprite
        ETG::Vector2f HoldPoint{};

        BOOST_DESCRIBE_CLASS(Character, (GameObjectBase),
                             (CurrentDir, AimAngle, IsShooting, CurrentGunIndex, HandOffsetRight, HandOffsetLeft,
                                 HandDepthInFront, HandDepthBehindBody),
                             (), ())

    protected:
        //Downcast helper for the derived classes' typed GetMoveComp(). Safe because each character builds its own
        //move component and never swaps it for another kind
        template <typename T>
        [[nodiscard]] T* GetMoveCompAs() const { return static_cast<T*>(MoveComp.get()); }

        //Called after EquipGun / SwitchGun settle on a new CurrentGun. The hero re-points its reload UI here;
        //an enemy has no UI and wants nothing
        virtual void OnGunChanged(GunBase* gun)
        {
        }

        //Whether the hand and the gun in it are drawn this frame. The hero hides them while dashing or reeling from
        //a hit (CanMove); BulletMan hides them only once it is dead, on the same rule it flips its sprites by
        [[nodiscard]] virtual bool ShouldShowHeldGun() const { return CanMove(); }
    };
}
