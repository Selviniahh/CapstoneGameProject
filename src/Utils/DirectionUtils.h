#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include "../Engine/Platform/Platform.h"
#include "../Engine/Core/Direction.h"

//What is left here is only what every character shares: turning an angle into a Direction. Anything that maps a
//Direction onto a *particular* character's animation keys lives next to that character
//(Game/Characters/HeroDirections.h, Game/Enemy/BulletMan/BulletManDirections.h).
//
//NOTE: This class used to hold all of those at once, which is why it included StateEnums.h - and since almost
//everything includes this header, every character's enums were in every translation unit. Its .cpp also included
//Hero.h, so a utility header dragged the player in behind it
namespace ETG
{
    struct PairHash
    {
        template <class T1, class T2>
        std::size_t operator()(const std::pair<T1, T2>& pair) const
        {
            auto hash1 = std::hash<T1>{}(pair.first);
            auto hash2 = std::hash<T2>{}(pair.second);
            return hash1 ^ (hash2 << 1);
        }
    };

    class DirectionUtils
    {
    public:
        //Key is the [min, max) degree arc, value is the Direction that arc means. Right straddles 0, so it is the
        //one direction that needs two entries
        using DirectionRanges = std::unordered_map<std::pair<float, float>, Direction, PairHash>;

        //The one table. Both the hero (mouse angle) and the enemies (angle to their target) read *this*.
        //
        //NOTE: There used to be two copies of it - this map, and a hand-written if/else chain inside
        //GetDirectionToTarget with its own set of boundary numbers. They agreed only because somebody kept them in
        //sync by hand, and editing the map in the editor left the enemies on the old one
        static DirectionRanges& GetRanges();

        //Overwrites the table with the default 45 degree arcs, each centred on its compass point. Called once on
        //first use; exposed so the editor can put a botched edit back
        static void ResetRangesToDefault();

        //Which arc the angle falls in. `angle` is in degrees and must already be normalised to [0, 360).
        //
        //NOTE: The bounds are half open on purpose. They used to be inclusive at both ends, so an angle landing
        //exactly on a boundary matched two arcs at once and the winner came down to unordered_map bucket order -
        //22 degrees resolved to the arc above it, 112 to the arc below it, both by accident
        static Direction GetDirectionFromAngle(float angle);

        //Which way `selfPosition` has to face to look at `targetPosition`. Used by the enemies to face the hero,
        //but it knows nothing about the hero - it takes two points
        static Direction GetDirectionToTarget(const ETG::Vector2f& targetPosition, const ETG::Vector2f& selfPosition);

        //Degrees in [0, 360) from `origin` to `target`, measured the way the rest of the game measures them:
        //0 is right, growing clockwise on screen
        static float GetAngleToTarget(const ETG::Vector2f& targetPosition, const ETG::Vector2f& origin);
    };
}
