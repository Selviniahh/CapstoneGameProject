#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "../Platform/Platform.h"

namespace ETG
{
    class GameObjectBase;

    // Runtime visualization for authored ETG::Vector2f members. Every Vector2f the reflection pass draws gets a
    // checkbox next to its input field; while that box is ticked, the point the member describes is drawn in the
    // world every frame as a colored cross. An offset like HandRig::ReloadReach.WorkingPoint{-9,2} stops being a pair
    // of numbers you have to picture against a sprite sheet and becomes a mark sitting on the gun.
    //
    // Markers are keyed on the address of the member itself, which is unique and stable for as long as the object
    // holding it lives. The owner is re-validated through GameClass::IsValid every frame, so a marker left on an
    // object that got destroyed drops itself instead of reading freed memory.
    class DebugPointViz
    {
    public:
        // Draws the color swatch and the toggle for one member. Owner may be null for values that do not live on a
        // game object (Animation's members, for instance) - there is no frame to anchor those to, so no toggle is
        // offered for them.
        static void Toggle(const char* label, ETG::Vector2f& value, GameObjectBase* owner);

        // Draws every enabled marker. Must be called inside the world sprite batch and after the objects have
        // drawn themselves, so a marker lands on top of the artwork it is annotating.
        static void DrawAll();

        // The reflection pass announces whose members it is about to walk, so Toggle knows what frame a point is
        // expressed in. ShowImGuiWidget<GameObjectBase*> recurses into nested objects, hence a stack and not a
        // single pointer.
        static void PushOwner(GameObjectBase* owner);
        static void PopOwner();
        static GameObjectBase* CurrentOwner();

    private:
        struct Marker
        {
            GameObjectBase* Owner;
            const ETG::Vector2f* Value;
            ETG::Color Color;
        };

        // Same member name always yields the same color, so a cross keeps its identity across sessions and across
        // being toggled off and on again
        static ETG::Color ColorFor(const char* label);

        // Only enabled markers are stored; unticking erases the entry, which keeps DrawAll's loop to what is
        // actually on screen
        static inline std::unordered_map<const void*, Marker> Markers{};
        static inline std::vector<GameObjectBase*> OwnerStack{};
    };
}
