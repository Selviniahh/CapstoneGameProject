#pragma once
#include <vector>
#include "../../../Engine/Platform/Platform.h"
#include "../../../Engine/Core/SingleInstance.h"
#include "../Character.h"
#include "HeroStates.h"
#include "../../../Engine/Core/Direction.h"
#include "../../../Engine/Core/Factory.h"
#include "../../Guns/Base/GunBase.h"
#include "HeroCapability.h"
#include "HeroStateMachine.h"
#include "../../Modifiers/ModifierManager.h"
#include "../../Modifiers/Hero/IHeroModifier.h"

namespace ETG
{
    class CollisionComponent;
    class ReloadText;
    class GunBase;
    class RogueSpecial;
    class HeroAnimComp;
    class InputComponent;
    class HeroMoveComp;
    class ReloadSlider;
    class ProjectileBase;

    //The playable character; anyone (components, enemies, items, UI) can reach the live one as Hero::GetSelf().
    //
    //NOTE: What is left here after Character took the body over is exactly the player-specific half: an input
    //component, a state machine driven by that input, and the reload UI hanging off the gun. A second playable
    //character derives from this and replaces its animations and stats, not its plumbing
    class Hero : public Character, public SingleInstance<Hero>
    {
    public:
        explicit Hero(ETG::Vector2f Position);
        ~Hero() override;
        void UpdateComponents();
        void UpdateAnimations();
        void HandleShooting() const;
        void HandleActiveItemInput() const;

        void Update() override;
        void Initialize() override;
        void Draw() override;
        void PopulateSpecificWidgets() override;

        //Character holds the move component as a BaseMoveComp; the dash lives on the hero's own type.
        //Defined in the .cpp: HeroMoveComp includes this header, so it cannot be complete here
        [[nodiscard]] HeroMoveComp* GetMoveComp() const;

        //Returns true if a modifier claimed the hit, in which case the caller must not apply any damage.
        //`projectile` is null for contact damage
        bool ConsumeIncomingDamage(ProjectileBase* projectile);

    public:
        //NOTE: The single owner of "what is the hero doing right now". Nothing outside this machine assigns a state;
        //the rest of the code either asks it a question or files a request through RequestDash / RequestHit
        std::unique_ptr<HeroStateMachine> StateMachine;

        std::unique_ptr<RogueSpecial> RogueSpecial;
        std::unique_ptr<ReloadText> ReloadText;

        ModifierManager<IHeroModifier> HeroModifierManager;

        std::unique_ptr<HeroAnimComp> AnimationComp;
        std::unique_ptr<InputComponent> InputComp;

        float HitKnockBackMagnitude = 150.f;
        float EnemyCollideKnockBackMag = 350.f;
        float HitForceDuration = 0.2f; //How long the knockback from taking a hit lasts

        //<---------- State queries ---------->
        //NOTE: There is deliberately no SetState. A state is entered when a transition's guard passes, never by
        //whoever happens to be running at the time
        [[nodiscard]] HeroStateEnum GetState() const { return StateMachine->GetActiveLeaf(); }

        //NOTE: Ask this instead of comparing GetState() against Die. It is a subtree question, so it stays correct
        //when the Dead branch grows a second leaf, and it lets the enemies stop speaking HeroStateEnum entirely
        [[nodiscard]] bool IsAlive() const override { return StateMachine->IsAlive(); }

        [[nodiscard]] bool CanSwitchGuns() const override { return CurrentGun && !CurrentGun->IsReloading && StateMachine->HasCapability(HeroCapability::CanSwitchGuns); }
        [[nodiscard]] bool CanMove() const override { return StateMachine->HasCapability(HeroCapability::CanMove); }
        [[nodiscard]] bool CanShoot() const override { return StateMachine->HasCapability(HeroCapability::CanShoot); }
        [[nodiscard]] bool CanFlipAnims() const override { return StateMachine->HasCapability(HeroCapability::CanFlipAnims); }
        [[nodiscard]] bool CanUseActiveItems() const override { return StateMachine->HasCapability(HeroCapability::CanUseActiveItems); }
        [[nodiscard]] bool CanTakeDamage() const override { return StateMachine->HasCapability(HeroCapability::CanTakeDamage); }

        //<---------- State requests ---------->
        //One-shot intents. The machine decides whether and when they become a state change, and clears them on entry
        void RequestDash(HeroDashEnum direction);
        void RequestHit(const ETG::Vector2f& knockbackDir, float forceMagnitude);

        //Drops whatever the machine did not act on this tick. Called right after Tick, so a request only ever gets
        //the frame it was filed on
        void ExpireRequests();

        bool DashRequested{};
        bool HitRequested{};
        HeroDashEnum CurrentDashDirection{HeroDashEnum::Unknown};
        ETG::Vector2f PendingKnockbackDir{};
        float PendingKnockbackForce{};

        BOOST_DESCRIBE_CLASS(Hero, (Character),
                             (HitKnockBackMagnitude, EnemyCollideKnockBackMag, HitForceDuration, CurrentDashDirection),
                             (),
                             ())

    protected:
        //The reload UI follows whichever gun is in hand
        void OnGunChanged(GunBase* gun) override;

        //Called from Hero's constructor only - see GameObjectBase::BindEvents
        void BindEvents() override;
    };
}