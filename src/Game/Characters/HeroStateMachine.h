#pragma once
#include "../../Engine/Core/StateMachine/HierarchicalStateMachine.h"
#include "HeroCapability.h"
#include "HeroStates.h"

namespace ETG
{
    class Hero;
   
    // HeroRoot                                      composite
    // │  Grants: -
    // │  Revokes: -
    // │
    // ├── Alive                                     composite
    // │   │  Grants: CanTakeDamage
    // │   │  Revokes: -
    // │   │
    // │   ├── Locomotion                            composite
    // │   │   │  Grants:
    // │   │   │    CanMove
    // │   │   │    CanShoot
    // │   │   │    CanSwitchGuns
    // │   │   │    CanUseActiveItems
    // │   │   │    CanFlipAnims
    // │   │   │  Revokes: -
    // │   │   │
    // │   │   ├── Idle                              leaf
    // │   │   │      Grants: -
    // │   │   │      Revokes: -
    // │   │   │      Effective: All
    // │   │   │
    // │   │   └── Run                               leaf
    // │   │          Grants: -
    // │   │          Revokes: -
    // │   │          Effective: All
    // │   │
    // │   ├── Dash                                  leaf
    // │   │      Grants: CanFlipAnims
    // │   │      Revokes: CanTakeDamage
    // │   │      Effective: CanFlipAnims
    // │   │
    // │   └── Hit                                   leaf
    // │          Grants: -
    // │          Revokes:
    // │            CanTakeDamage
    // │            CanFlipAnims
    // │          Effective: None
    // │
    // └── Dead                                      composite
    //     │  Grants: -
    //     │  Revokes: All
    //     │
    //     └── Die                                   leaf
    //            Grants: -
    //            Revokes: -
    //            Effective: None
    
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
        Node* NormalMovement{};
        Node* IdleNode{};
        Node* RunNode{};
        Node* DashNode{};
        Node* HitNode{};
        Node* DeadNode{};
        Node* DieNode{};

        [[nodiscard]] bool IsAlive() const { return IsInNode(AliveNode); }
        [[nodiscard]] bool IsOnFoot() const { return IsInNode(NormalMovement); }
    };
}
