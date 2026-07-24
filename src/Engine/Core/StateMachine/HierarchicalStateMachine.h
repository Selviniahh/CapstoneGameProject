#pragma once
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include "StateNode.h"
#include "../Events/EventDelegate.h"

//Hierarchical finite state machine.
//
//Why hierarchical instead of a flat table: a character's states are not a flat list, they are nested categories.
//"Idle and Run both allow shooting" and "Hit and Die interrupt everything alive" are statements about *groups* of
//states. A flat enum has nowhere to put that, so those statements end up re-typed as bitmasks and as
//`if (state != Die && state != Hit)` guards at every call site. Here they are declared once on a parent node and
//every descendant inherits them.
//
//Two rules do all the work:
//  1. Transitions are evaluated root -> leaf, so an outer interrupt always beats an inner routine.
//  2. A node with no outgoing transitions is terminal, so "you cannot come back from death" is structural.
namespace ETG
{
    template <typename StateEnum, typename OwnerT, typename CapabilityT>
    class HierarchicalStateMachine
    {
    public:
        using Node = StateNode<StateEnum, OwnerT, CapabilityT>;
        using CapabilityBits = std::underlying_type_t<CapabilityT>;

        //If a tick ever needs more than this many transitions to settle, the tree has a cycle
        static constexpr int MaxTransitionsPerTick = 8;

        //from, to
        EventDelegate<StateEnum, StateEnum> OnStateChanged;

        //<---------- Tree construction ---------->

        //The machine owns every node it hands out, so the tree lives exactly as long as the machine does
        Node* CreateNode(std::string name)
        {
            Nodes.push_back(std::make_unique<Node>(std::move(name)));
            return Nodes.back().get();
        }

        Node* CreateLeaf(std::string name, StateEnum leafId)
        {
            Nodes.push_back(std::make_unique<Node>(std::move(name), leafId));
            return Nodes.back().get();
        }

        void SetRoot(Node* root) { Root = root; }
        [[nodiscard]] Node* GetRoot() const { return Root; }

        //<---------- Runtime ---------->

        //Enter the tree for the first time. Must be called before Tick
        void Start(OwnerT& owner)
        {
            if (!Root) throw std::runtime_error("HierarchicalStateMachine::Start called without a root node");

            ActivePath.clear();
            ActivePath.push_back(Root);
            if (Root->OnEnter) Root->OnEnter(owner);
            DescendToLeaf(owner);

            TimeInCurrentState = 0.f;
            CachedLeaf = ReadActiveLeaf();
        }

        void Tick(OwnerT& owner, const float deltaTime)
        {
            if (ActivePath.empty()) throw std::runtime_error("HierarchicalStateMachine::Tick called before Start");

            TimeInCurrentState += deltaTime;

            //At most one *new* decision per tick, then the tree is allowed to settle inwards.
            //
            //NOTE: The settling pass is restricted to nodes inside whatever we just entered, and that restriction is
            //load bearing. Entering a composite lands on its default child, and that child may immediately have a
            //reason to move on (leaving Dash lands on Idle, but the player is still holding a movement key) - that
            //has to resolve now or the hero renders one frame of the wrong animation. What must NOT happen is a
            //second unrelated interrupt firing in the same tick: with two requests pending, the hero would enter Hit
            //and leave for Dash before the hit animation ever drew a frame. Anything declared outside the subtree we
            //just entered is a fresh decision and waits for the next tick
            const Node* settleWithin = nullptr;
            for (int i = 0; i < MaxTransitionsPerTick; ++i)
            {
                const TransitionMatch match = FindTransitionMatch(owner, settleWithin);
                if (!match.Target) break;

                TransitionTo(owner, match.Target);
                settleWithin = match.Target;
            }

            //Tick root -> leaf so outer behaviour runs before the more specific one
            for (Node* node : ActivePath)
                if (node->OnTick) node->OnTick(owner, deltaTime);
        }

        //<---------- Queries ---------->

        //The enum the rest of the game still speaks. Valid even before Start (reports the default-constructed state)
        [[nodiscard]] StateEnum GetActiveLeaf() const { return CachedLeaf; }

        [[nodiscard]] const Node* GetActiveNode() const { return ActivePath.empty() ? nullptr : ActivePath.back(); }

        //True when every requested capability is granted somewhere on the active path and revoked nowhere on it
        [[nodiscard]] bool HasCapability(const CapabilityT capability) const
        {
            const auto requested = static_cast<CapabilityBits>(capability);
            if (requested == 0) return true;

            CapabilityBits granted{};
            CapabilityBits revoked{};
            for (const Node* node : ActivePath)
            {
                granted |= static_cast<CapabilityBits>(node->Grants);
                revoked |= static_cast<CapabilityBits>(node->Revokes);
            }

            return ((granted & ~revoked) & requested) == requested;
        }

        //True when the given node is the active leaf or one of its ancestors. Lets callers ask about a whole subtree
        //("am I anywhere inside Alive?") instead of enumerating leaves
        [[nodiscard]] bool IsInNode(const Node* node) const
        {
            return std::find(ActivePath.begin(), ActivePath.end(), node) != ActivePath.end();
        }

        //Seconds since the active leaf was last entered. Replaces the per-component dash/hit timers
        [[nodiscard]] float TimeInState() const { return TimeInCurrentState; }

        //Root -> leaf, for debugging and the editor UI
        [[nodiscard]] const std::vector<Node*>& GetActivePath() const { return ActivePath; }

        [[nodiscard]] std::string GetActivePathName() const
        {
            std::string result;
            for (const Node* node : ActivePath)
            {
                if (!result.empty()) result += '/';
                result += node->Name;
            }
            return result;
        }

    private:
        struct TransitionMatch
        {
            const Node* Declarer{}; //The node the transition was declared on
            Node* Target{};
        };

        //Walks root -> leaf and returns the first transition whose guard passes. Evaluating the outermost node first
        //is what gives interrupts their priority for free: Die is declared on Alive, Idle<->Run deep inside Locomotion.
        //Within one node, declaration order decides.
        //When `settleWithin` is set, only transitions declared on that node or one of its descendants are considered
        TransitionMatch FindTransitionMatch(const OwnerT& owner, const Node* settleWithin) const
        {
            for (const Node* node : ActivePath)
            {
                if (settleWithin && !IsDescendantOrSelf(node, settleWithin)) continue;

                for (const auto& transition : node->Transitions)
                    if (transition.Target && transition.Condition && transition.Condition(owner))
                        return TransitionMatch{node, transition.Target};
            }

            return {};
        }

        static bool IsDescendantOrSelf(const Node* node, const Node* ancestor)
        {
            for (; node; node = node->Parent)
                if (node == ancestor) return true;

            return false;
        }

        void TransitionTo(OwnerT& owner, Node* target)
        {
            const StateEnum previousLeaf = CachedLeaf;
            Node* const commonAncestor = FindLowestCommonAncestor(ActivePath.back(), target);

            //Exit leaf -> ancestor, stopping below the common ancestor because that part of the path is not left
            for (auto it = ActivePath.rbegin(); it != ActivePath.rend() && *it != commonAncestor; ++it)
                if ((*it)->OnExit) (*it)->OnExit(owner);

            while (!ActivePath.empty() && ActivePath.back() != commonAncestor) ActivePath.pop_back();

            //Enter ancestor -> target, top down
            std::vector<Node*> entryChain;
            for (Node* node = target; node && node != commonAncestor; node = node->Parent) entryChain.push_back(node);
            std::reverse(entryChain.begin(), entryChain.end());

            for (Node* node : entryChain)
            {
                ActivePath.push_back(node);
                if (node->OnEnter) node->OnEnter(owner);
            }

            DescendToLeaf(owner);

            TimeInCurrentState = 0.f;
            CachedLeaf = ReadActiveLeaf();
            OnStateChanged.Broadcast(previousLeaf, CachedLeaf);
        }

        //A composite node is never "the" state. Keep walking its default child until an actual leaf is reached
        void DescendToLeaf(OwnerT& owner)
        {
            while (!ActivePath.back()->IsLeaf())
            {
                Node* const child = ActivePath.back()->DefaultChild;
                if (!child) throw std::runtime_error("Composite state '" + ActivePath.back()->Name + "' has no default child");

                ActivePath.push_back(child);
                if (child->OnEnter) child->OnEnter(owner);
            }
        }

        static Node* FindLowestCommonAncestor(Node* lhs, Node* rhs)
        {
            if (!lhs || !rhs) return nullptr;

            int lhsDepth = lhs->Depth();
            int rhsDepth = rhs->Depth();

            while (lhsDepth > rhsDepth) { lhs = lhs->Parent; --lhsDepth; }
            while (rhsDepth > lhsDepth) { rhs = rhs->Parent; --rhsDepth; }

            while (lhs != rhs)
            {
                lhs = lhs->Parent;
                rhs = rhs->Parent;
            }

            return lhs;
        }

        [[nodiscard]] StateEnum ReadActiveLeaf() const
        {
            const Node* leaf = ActivePath.back();
            if (!leaf->LeafId.has_value()) throw std::runtime_error("Leaf state '" + leaf->Name + "' was declared without an enum id");

            return *leaf->LeafId;
        }

        std::vector<std::unique_ptr<Node>> Nodes;
        Node* Root{};

        //Root -> leaf
        std::vector<Node*> ActivePath;

        //Kept in sync with ActivePath.back() so GetActiveLeaf stays O(1) and is safe to call before Start
        StateEnum CachedLeaf{};
        float TimeInCurrentState{};
    };
}
