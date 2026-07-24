#pragma once
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

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
        using Guard = std::function<bool(const OwnerT&)>;
        using Action = std::function<void(OwnerT&)>;
        using TickAction = std::function<void(OwnerT&, float deltaTime)>;

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
            for (const StateNode* node = Parent; node; node = node->Parent) ++depth;
            return depth;
        }
    };
}
