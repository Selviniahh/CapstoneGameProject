#include "HeroStateMachine.h"
#include "Hero.h"
#include "Components/HeroAnimComp.h"
#include "Components/HeroMoveComp.h"
#include "../../../Engine/Core/Components/BaseHealthComp.h"
#include "../../../Engine/Managers/InputManager.h"

namespace ETG
{
    void HeroStateMachine::Build()
    {
        using Cap = HeroCapability;

        RootNode = CreateNode("HeroRoot");
        AliveNode = CreateNode("Alive");
        NormalMovement = CreateNode("Locomotion");
        
        IdleNode = CreateLeaf("Idle", HeroStateEnum::Idle);
        RunNode = CreateLeaf("Run", HeroStateEnum::Run);
        DashNode = CreateLeaf("Dash", HeroStateEnum::Dash);
        HitNode = CreateLeaf("Hit", HeroStateEnum::Hit);
        DeadNode = CreateNode("Dead");
        DieNode = CreateLeaf("Die", HeroStateEnum::Die);

        //<---------- Shape ---------->
        //The first child attached is the default one, so entering Alive lands in Locomotion/Idle
        RootNode->AddChild(AliveNode);
        RootNode->AddChild(DeadNode);
        AliveNode->AddChild(NormalMovement);
        AliveNode->AddChild(DashNode);
        AliveNode->AddChild(HitNode);
        NormalMovement->AddChild(IdleNode);
        NormalMovement->AddChild(RunNode);
        DeadNode->AddChild(DieNode);
        SetRoot(RootNode);

        //<---------- Capabilities ---------->
        //3 farkli kural var: 
        //Capabilities → Karakter ne yapabilir?
        // Transitions  → Ne zaman başka state'e geçer?
        // Actions      → State aktifken ne yapılır?
        //Declared once, on the node that owns the rule. Idle and Run declare nothing: they inherit from Locomotion
        AliveNode->Grants = Cap::CanTakeDamage;
        NormalMovement->Grants = Cap::CanMove | Cap::CanShoot | Cap::CanSwitchGuns | Cap::CanUseActiveItems | Cap::CanFlipAnims;
        DashNode->Grants = Cap::CanFlipAnims;
        DashNode->Revokes = Cap::CanTakeDamage;
        HitNode->Revokes = Cap::CanTakeDamage | Cap::CanFlipAnims;

        //Nothing on the Dead path grants anything, so this is belt and braces. It is also the clearest way to tell
        //the next reader that a corpse can do nothing
        DeadNode->Revokes = Cap::All;

        //<---------- Transitions ---------->
        // Ne zaman başka state'e geçer?
        //NOTE: Transitions are evaluated root -> leaf, and within a node in declaration order. That ordering IS the
        //priority. Death beats being hit, being hit beats dashing, and all three beat walking around, without a
        //single `if (state != Die && state != Hit)` anywhere in the codebase

        //Declared on Alive, so it fires from Idle, Run, Dash and Hit alike
        AliveNode->AddTransition(DeadNode, [](const Hero& hero)
        {
            return hero.HealthComp && hero.HealthComp->IsDead();
        }, "Alive -> Dead");

        AliveNode->AddTransition(HitNode, [](const Hero& hero)
        {
            return hero.HitRequested;
        }, "Alive -> Hit");

        AliveNode->AddTransition(DashNode, [this](const Hero& hero)
        {
            //NOTE: this transition hangs off Alive so it fires from Idle, Run and Hit alike - which also meant it
            //fired from Dash itself. Input re-files DashRequested on every frame the button is held, and the
            //cooldown does not start until Dash *exits*, so IsDashAvailable() was still true mid-dash: holding the
            //button re-entered Dash every frame and restarted the dash before the first one had finished.
            //Chaining still works, it just has to wait for the current dash to end
            if (IsInNode(DashNode)) return false;

            return hero.DashRequested && hero.GetMoveComp() && hero.GetMoveComp()->IsDashAvailable();
        }, "Alive -> Dash");

        //NOTE: TimeInState() is 0 for the whole entry frame, which stops these from firing before the animation
        //component has had its chance to restart the animation they are waiting on
        HitNode->AddTransition(NormalMovement, [this](const Hero& hero)
        {
            return TimeInState() > 0.f && hero.AnimationComp->AnimManagerDict[HeroStateEnum::Hit].IsAnimationFinished();
        }, "Hit -> Locomotion");

        DashNode->AddTransition(NormalMovement, [this](const Hero& hero)
        {
            if (TimeInState() < hero.GetMoveComp()->MinDashDuration) return false;
            return hero.AnimationComp->AnimManagerDict[HeroStateEnum::Dash].IsAnimationFinished();
        }, "Dash -> Locomotion");

        IdleNode->AddTransition(RunNode, [](const Hero&) { return InputManager::IsMoving(); }, "Idle -> Run");
        RunNode->AddTransition(IdleNode, [](const Hero&) { return !InputManager::IsMoving(); }, "Run -> Idle");

        //Dead declares no transitions at all. Resurrection is not blocked by a check, it is simply unreachable

        //<---------- Actions ---------->
        HitNode->OnEnter = [](Hero& hero)
        {
            //The knockback the damage listener asked for. Applying it here means the force and the state can never
            //disagree, which they could while the listener did both jobs itself
            hero.GetMoveComp()->ApplyForce(hero.PendingKnockbackDir, hero.PendingKnockbackForce, hero.HitForceDuration);
            hero.HitRequested = false;
        };

        DashNode->OnEnter = [](Hero& hero)
        {
            hero.GetMoveComp()->BeginDash();
            hero.DashRequested = false;
        };

        DashNode->OnTick = [this](Hero& hero, float)
        {
            hero.GetMoveComp()->MakeDashMovement(TimeInState());
        };

        DashNode->OnExit = [](Hero& hero)
        {
            hero.GetMoveComp()->StartDashCooldown();
        };

        NormalMovement->OnTick = [](Hero& hero, float)
        {
            hero.GetMoveComp()->UpdateMovement();
        };

        DieNode->OnTick = [](Hero& hero, float)
        {
            //Hold the last frame forever. PlayOnlyLastFrame is idempotent, so running it every tick is fine
            auto& deathAnim = hero.AnimationComp->AnimManagerDict[HeroStateEnum::Die];
            if (deathAnim.IsAnimationFinished() && deathAnim.CurrentAnim) deathAnim.CurrentAnim->PlayOnlyLastFrame();
        };
    }
}
