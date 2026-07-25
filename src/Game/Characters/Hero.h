#pragma once
#include <vector>
#include "../../Engine/Platform/Platform.h"
#include "../../Engine/Core/GameObjectBase.h"
#include "../../Engine/Core/SingleInstance.h"
#include "HeroStates.h"
#include "../../Engine/Core/Direction.h"
#include "../../Engine/Core/Factory.h"
#include "../Guns/Base/GunBase.h"
#include "HeroCapability.h"
#include "HeroStateMachine.h"

namespace ETG
{
    class CollisionComponent;
    class ReloadText;
    class GunBase;
    class ActiveItemBase;
    class PassiveItemBase;
    class Hand;
    class RogueSpecial;
    class HeroAnimComp;
    class InputComponent;
    class HeroMoveComp;
    class ReloadSlider;
    class BaseHealthComp;

    //The single player character; anyone (components, enemies, items, UI) can reach it as Hero::GetSelf()
    class Hero : public GameObjectBase, public SingleInstance<Hero>
    {
    public:
        explicit Hero(ETG::Vector2f Position);
        ~Hero() override;
        void UpdateComponents();
        void UpdateAnimations();
        void UpdateHand() const;
        void UpdateGuns() const;
        void HandleShooting() const;
        void HandleActiveItem() const;

        void Update() override;
        void Initialize() override;
        void Draw() override;
        void PopulateSpecificWidgets() override;
        [[nodiscard]] GunBase* GetCurrentHoldingGun() const;

    public:
        static float MouseAngle;
        static Direction CurrentDirection;
        static bool IsShooting;

        //NOTE: The single owner of "what is the hero doing right now". Nothing outside this machine assigns a state;
        //the rest of the code either asks it a question or files a request through RequestDash / RequestHit
        std::unique_ptr<HeroStateMachine> StateMachine;

        std::unique_ptr<RogueSpecial> RogueSpecial;
        std::unique_ptr<HeroMoveComp> MoveComp;
        std::unique_ptr<Hand> Hand;
        std::unique_ptr<ReloadText> ReloadText;
        std::unique_ptr<CollisionComponent> CollisionComp;
        std::unique_ptr<BaseHealthComp> HealthComp;

        ActiveItemBase* CurrActiveItem{};

        //Items the hero has picked up (non-owning; the items live in the world object list).
        //Items register themselves here on pickup, UI reads these to draw the equipped item slots.
        std::vector<ActiveItemBase*> EquippedActiveItems;
        std::vector<PassiveItemBase*> EquippedPassiveItems;

        std::unique_ptr<HeroAnimComp> AnimationComp;
        std::unique_ptr<InputComponent> InputComp;

        //Selected guns 
        std::vector<GunBase*> EquippedGuns; // Array of equipped guns
        GunBase* CurrentGun = nullptr; // Currently selected gun
        int currentGunIndex = 0; // Track the index of current gun
        float HitKnockBackMagnitude = 150.f;
        float EnemyCollideKnockBackMag = 350.f;
        float HitForceDuration = 0.2f; //How long the knockback from taking a hit lasts

        //<---------- State queries ---------->
        //NOTE: There is deliberately no SetState. A state is entered when a transition's guard passes, never by
        //whoever happens to be running at the time
        [[nodiscard]] HeroStateEnum GetState() const { return StateMachine->GetActiveLeaf(); }

        //NOTE: Ask this instead of comparing GetState() against Die. It is a subtree question, so it stays correct
        //when the Dead branch grows a second leaf, and it lets the enemies stop speaking HeroStateEnum entirely
        [[nodiscard]] bool IsAlive() const { return StateMachine->IsAlive(); }

        [[nodiscard]] inline bool CanSwitchGuns() const { return !CurrentGun->IsReloading && StateMachine->HasCapability(HeroCapability::CanSwitchGuns); }
        [[nodiscard]] inline bool CanMove() const { return StateMachine->HasCapability(HeroCapability::CanMove); }
        [[nodiscard]] inline bool CanShoot() const { return StateMachine->HasCapability(HeroCapability::CanShoot); }
        [[nodiscard]] inline bool CanFlipAnims() const { return StateMachine->HasCapability(HeroCapability::CanFlipAnims); }
        [[nodiscard]] inline bool CanUseActiveItems() const { return StateMachine->HasCapability(HeroCapability::CanUseActiveItems); }
        [[nodiscard]] inline bool CanTakeDamage() const { return StateMachine->HasCapability(HeroCapability::CanTakeDamage); }

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

        //When equipping a new gun pickup
        void EquipGun(GunBase* newGun);
        void SwitchGun(const int& index);

        // When scrolling the mouse wheel, switch back to the default (index 0) gun.
        void SwitchToPreviousGun();
        void SwitchToNextGun();

        BOOST_DESCRIBE_CLASS(Hero, (GameObjectBase),
                             (MouseAngle, CurrentDirection, IsShooting, HitKnockBackMagnitude, HitForceDuration, CurrentDashDirection),
                             (),
                             ())

    private:
        void UpdateGunVisibility() const;
    };
}
