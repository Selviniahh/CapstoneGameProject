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
        //Left out of Update() deliberately: the two branches run these in different orders against their own state
        //machinery, and that ordering is the part they do not agree on
        void UpdateHoldPoint();
        void UpdateGuns();
        void UpdateHandAndGunVisibility() const;

        //Which hand the current gun is in, and therefore which way its sprite is mirrored. The gun answers this
        //itself when it names a GunBase::HandSwapAngle - a revolver flipping exactly at vertical - and otherwise
        //the body's 8-way facing decides. One answer, read by the hold point, both hands and the gun's flip, so
        //they cannot turn over at different angles
        [[nodiscard]] bool IsGunOnRightSide() const;

        //Honours a gun that asked to turn about its left-hand grip instead of about its own origin, by sliding the
        //whole gun so that grip lands back on its pinned point. Called from UpdateGuns once the gun's rotation and
        //mirror are settled, and before the hands are placed on it - so the hands ride the pin for free
        void ApplyGripPin() const;

        //Both hands land on the pixels the current gun named for them. Called from UpdateGuns, after the guns have
        //ticked, because it reads the Origin their animations just wrote
        void UpdateHands() const;

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
