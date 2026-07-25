#pragma once
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

/* - Detayli Aciklama 
 * Bir durumdasın.
 - O durumdan başka bir duruma giden geçişler var.
 - Geçişin koşulu true olursa hedef duruma geçiyorsun.

 Fakat buradaki sistem yalnızca animasyonu değil, karakterin genel oyun durumunu da yönetiyor: hareket edebilir mi, ateş edebilir mi, hasar alabilir mi gibi.

 Unreal karşılıkları kabaca şöyle:

  Unreal’daki kavram          Buradaki karşılığı
 ━━━━━━━━━━━━━━━━━━━━━━━━━━  ━━━━━━━━━━━━━━━━━━━━
  Idle, Run, Dash kutuları    Leaf node
 ──────────────────────────  ────────────────────
  Geçiş oku                   Transition
 ──────────────────────────  ────────────────────
  Okun içindeki koşul         Guard / Condition
 ──────────────────────────  ────────────────────
  Okun hedefindeki state      Transition::Target
 ──────────────────────────  ────────────────────
  İç içe state grubu          Composite node


Node, ağaçtaki herhangi bir kutudur. İki çeşit olabilir:

  Node
  ├── Composite node
  └── Leaf node 
 
Leaf nedir?

  Leaf, altında başka çocuk bulunmayan node’dur:

  [[nodiscard]] bool IsLeaf() const
  {
      return Children.empty();
  }
  
Root
  └── Alive
      └── Locomotion
          └── Run  ← leaf
          
Örnek olarak Unreal’daki şu yapı:
  Idle ──[karakter hareket ediyor]──> Run
  Burada şöyle temsil edilebilir:

  idle->AddTransition(
      run,
      [](const Hero& hero)
      {
          return hero.IsMoving();
      },
      "Start Moving"
  );

  Parçaları:

  idle                 // Başlangıç state'i
  run                  // Okun hedefi
  hero.IsMoving()      // Okun condition'ı
  "Start Moving"       // Geçişin debug adı

  En kısa özet:

  Idle / Run / Dash = Leaf
  Ok                 = Transition
  Okun koşulu         = Guard / Condition
  Alive / Locomotion  = Composite Node
  Node                = Bunların hepsinin genel adı


Composite node, birden fazla state’i ortak kurallar altında gruplamak için kullanılır. Kendisi gerçek son state değildir; altında başka node’lar bulunur.

  Projendeki ağaç:

  HeroRoot                         composite
  ├── Alive                       composite
  │   ├── Locomotion              composite
  │   │   ├── Idle                leaf
  │   │   └── Run                 leaf
  │   ├── Dash                    leaf
  │   └── Hit                     leaf
  └── Dead                        composite
      └── Die                     leaf

  Örneğin karakter Run durumundaysa aktif yol şudur:

  HeroRoot → Alive → Locomotion → Run

  Run leaf’tir. Diğerleri onun aktif composite atalarıdır.

  ### Ortak özellik vermek

  Idle ve Run aynı yeteneklere sahip:

  LocomotionNode->Grants =
      Cap::CanMove |
      Cap::CanShoot |
      Cap::CanSwitchGuns |
      Cap::CanUseActiveItems |
      Cap::CanFlipAnims;

  Bunları ayrı ayrı yazmak gerekmiyor:

  IdleNode->Grants = ...;
  RunNode->Grants = ...;

  Çünkü Idle ve Run, Locomotion altında oldukları için bu yetenekleri miras alıyor.

  ### Ortak davranış vermek

  Hem Idle hem de Run sırasında hareket sistemi güncellensin:

  LocomotionNode->OnTick = [](Hero& hero, float)
  {
      hero.MoveComp->UpdateMovement();
  };

  Aktif leaf Idle veya Run olduğunda LocomotionNode->OnTick çalışır.

  ### Birçok state’e tek transition vermek

  Şu transition Alive üzerine eklenmiş:

  AliveNode->AddTransition(DeadNode, [](const Hero& hero)
  {
      return hero.HealthComp && hero.HealthComp->IsDead();
  });

  Bunun anlamı:

  Idle ──┐
  Run  ──┤
  Dash ──┼── karakter öldü ──> Dead/Die
  Hit  ──┘

  Dört ayrı transition yazmak yerine Alive composite node’una bir tane yazılıyor.

  ### Composite’e geçiş

  Dash bittikten sonra:

  DashNode->AddTransition(LocomotionNode, ...);

  Hedef Locomotion composite node’u. Composite gerçek son state olmadığı için sistem onun varsayılan çocuğuna iner:

  Locomotion → Idle

  Projede ilk eklenen çocuk otomatik olarak varsayılan çocuk oluyor:

  LocomotionNode->AddChild(IdleNode); // DefaultChild
  LocomotionNode->AddChild(RunNode);

  Kısacası:

  Leaf:
  Gerçek durumdur.
  Idle, Run, Dash, Hit, Die

  Composite:
  State’leri gruplar.
  Ortak capability, transition ve davranış taşır.
  Alive, Locomotion, Dead

  Composite node’u “klasör” gibi düşünebilirsin; fakat sadece düzenleme yapmaz. İçindeki tüm state’lere ortak kurallar ve davranışlar da kazandırır.

*/

//A single node of a hierarchical state machine tree.
//Leaf nodes carry a StateEnum value, so the rest of the codebase keeps addressing states by enum: AnimManagerDict,
//the AnimationKey variant and the boost::describe editor UI are all keyed on those enums and stay untouched.
//Composite nodes carry no enum; they exist to hold capabilities and transitions that every descendant inherits.
namespace ETG
{
    template <typename StateEnum, typename OwnerT, typename CapabilityT>
    struct StateNode
    {
        //A guard is asked "should we leave for this target?" and must not mutate anything
        using Guard = std::function<bool(const OwnerT&)>; //okun icindeki kosul 
        using Action = std::function<void(OwnerT&)>; //bir state’e girildiğinde veya state’ten çıkıldığında çalışan event
        using TickAction = std::function<void(OwnerT&, float deltaTime)>; //State aktifken her frame çalışan update

        //unrealdaki Geçiş oku
        struct Transition
        {
            StateNode* Target{};
            Guard Condition{};
            std::string Name{}; //Only for debugging / UI
        };

        explicit StateNode(std::string name) : Name(std::move(name))
        {
        }

        StateNode(std::string name, StateEnum leafId) : Name(std::move(name)), LeafId(leafId)
        {
        }

        std::string Name;

        //Only leaves have a value. This is what GetActiveLeaf() reports back to the rest of the game
        //"home/selviniah/CLionProjects/EnterTheGungeonClone/docs/Later/std::optional.md"
        std::optional<StateEnum> LeafId{};

        StateNode* Parent{};
        std::vector<StateNode*> Children{};

        //Entering a composite node means descending into this child until a leaf is reached
        StateNode* DefaultChild{};

        Action OnEnter{};
        Action OnExit{};
        TickAction OnTick{};

        //Capabilities are accumulated along the active path from root to leaf. Revokes always beat Grants,
        //so a parent can hand out a capability and a single child can take it away without the parent knowing
        CapabilityT Grants{};
        CapabilityT Revokes{};

        //NOTE: A transition declared here is evaluated while this node OR any of its descendants is active.
        //That is the whole point of the hierarchy: "death interrupts anything alive" is one declaration on Alive
        //instead of a `!= Die` check repeated at every call site that assigns a state
        std::vector<Transition> Transitions{};

        //The first child attached becomes the default one unless DefaultChild is set explicitly afterwards
        StateNode* AddChild(StateNode* child)
        {
            child->Parent = this;
            Children.push_back(child);
            if (!DefaultChild) DefaultChild = child;
            return child;
        }

        void AddTransition(StateNode* target, Guard condition, std::string name = {})
        {
            Transitions.push_back(Transition{target, std::move(condition), std::move(name)});
        }

        [[nodiscard]] bool IsLeaf() const { return Children.empty(); }

        [[nodiscard]] int Depth() const
        {
            int depth = 0;
            for (
                const StateNode* node = Parent; // Başlangıç
                node != nullptr; // Devam koşulu
                node = node->Parent // Her turdan sonra
            )
            {
                ++depth;
            }
            return depth;
        }
    };
}
