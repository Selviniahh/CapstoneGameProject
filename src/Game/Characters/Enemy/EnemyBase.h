#pragma once
#include "../Character.h"
#include "../Hero/Hero.h"
#include "../../../Engine/Core/Events/EventDelegate.h"
#include "../../Managers/Enum/StateFlags.h"
#include "EnemyStates.h"
#include "Components/EnemyMoveCompBase.h"

namespace ETG
{
    class ProjectileBase;
    class Hero;
    class Hand;
    class RogueSpecial;
    class BaseHealthComp;
    class CollisionComponent;
    class EnemyMoveCompBase;

    //Everything an enemy shares with the hero (body, hand, gun inventory, item bag) now comes from Character.
    //What is left here is the enemy's own answer to "what am I doing": a flat state enum plus the flag set that
    //turns it into permissions - the counterpart of the hero's state machine
    class EnemyBase : public Character
    {
    protected:
        EnemyBase(); // Add default constructor
        ~EnemyBase() override;
        void Initialize() override;
        void Update() override;
        void Draw() override;

    public:
        // Projectile collision handling
        virtual void HandleHitForce(const ProjectileBase* projectile);

        //Character holds the move component as a BaseMoveComp; the chase AI lives on the enemy's own type
        [[nodiscard]] EnemyMoveCompBase* GetMoveComp() const { return GetMoveCompAs<EnemyMoveCompBase>(); }

        //<---------- Capabilities ---------->
        //The same questions the hero answers from its state tree, answered here from the flat flag set
        [[nodiscard]] bool IsAlive() const override { return EnemyState != EnemyStateEnum::Die; }
        [[nodiscard]] bool CanMove() const override { return !HasAnyFlag(StateFlags, EnemyStateFlag::PreventMovement); }
        [[nodiscard]] bool CanShoot() const override { return !HasAnyFlag(StateFlags, EnemyStateFlag::PreventShooting) && Hero->IsAlive(); }
        [[nodiscard]] bool CanFlipAnims() const override { return !HasAnyFlag(StateFlags, EnemyStateFlag::PreventAnimFlip); }
        [[nodiscard]] bool CanTakeDamage() const override { return !HasAnyFlag(StateFlags, EnemyStateFlag::PreventTakingDamage); }
        [[nodiscard]] bool CanSwitchGuns() const override { return !HasAnyFlag(StateFlags, EnemyStateFlag::PreventGunSwitch) && (!CurrentGun || !CurrentGun->IsReloading); }
        [[nodiscard]] bool CanUseActiveItems() const override { return !HasAnyFlag(StateFlags, EnemyStateFlag::PreventItemUse); }

        void SetState(const EnemyStateEnum& state);
        [[nodiscard]] EnemyStateEnum GetState() const { return EnemyState; }

        float KnockBackMagnitudeForDeath = 75;
        float KnockBackDurationForDeath = 1.0f;

    private:
        EnemyStateEnum EnemyState{EnemyStateEnum::Idle};

    protected:
        Hero* Hero;
        EnemyStateFlag StateFlags{EnemyStateFlag::StateIdle};

        //TODO: This is stupid, remove this somehow I think the gun must decide how force duration should be so move this into the GunBase  
        float ForceDurationDivider = 6.f;

        //Enemies keep the sprite flip in step with their sprites the way BulletMan always did: the hand and gun go
        //away only once the enemy is dead, not while it is reeling from a hit
        [[nodiscard]] bool ShouldShowHeldGun() const override { return CanFlipAnims(); }

        BOOST_DESCRIBE_CLASS(EnemyBase, (Character), (KnockBackMagnitudeForDeath, KnockBackDurationForDeath), (Hero, ForceDurationDivider), (EnemyState))
    };
}