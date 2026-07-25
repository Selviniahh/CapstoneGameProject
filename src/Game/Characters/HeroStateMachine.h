#pragma once
#include "../../Engine/Core/StateMachine/HierarchicalStateMachine.h"
#include "../Managers/Enum/HeroCapability.h"
#include "HeroStates.h"

namespace ETG
{
    class Hero;

    //The hero's state tree. Every transition the hero can make is declared in Build(), in one readable block,
    //instead of being spread across SetState() calls in the move component, the anim component and a health listener.
    //
    //  HeroRoot
    //  |- Alive                    grants CanTakeDamage
    //  |  |- Locomotion            grants CanMove | CanShoot | CanSwitchGuns | CanUseActiveItems | CanFlipAnims
    //  |  |  |- Idle   (default)
    //  |  |  '- Run
    //  |  |- Dash                  grants CanFlipAnims, revokes CanTakeDamage
    //  |  '- Hit                   revokes CanTakeDamage and CanFlipAnims
    //  '- Dead                     terminal, declares no outgoing transitions
    //     '- Die
    
    //Final HeroStateMachine sınıfından başka bir sınıfın türetilmesini engeller.
                                                                //Once animasyon karakter, kurallar 
    class HeroStateMachine final : public HierarchicalStateMachine<HeroStateEnum, Hero, HeroCapability>
    {
    public:
        //Builds the tree. Safe to call before the hero's components exist: guards and actions only run from Tick,
        //and no node on the initial path has an OnEnter that touches a component
        void Build();

        //Kept so callers can ask about a whole subtree instead of enumerating leaves
        Node* RootNode{};
        Node* AliveNode{};
        Node* LocomotionNode{};
        Node* IdleNode{};
        Node* RunNode{};
        Node* DashNode{};
        Node* HitNode{};
        Node* DeadNode{};
        Node* DieNode{};

        [[nodiscard]] bool IsAlive() const { return IsInNode(AliveNode); }
        [[nodiscard]] bool IsOnFoot() const { return IsInNode(LocomotionNode); }
    };
}
