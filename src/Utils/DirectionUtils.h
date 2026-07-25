#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include "../Engine/Platform/Platform.h"
#include "../Engine/Core/Direction.h"

//What is left here is only what every character shares: turning an angle into a Direction, and the range map that
//does it. Anything that maps a Direction onto a *particular* character's animation keys now lives next to that
//character (Game/Characters/HeroDirections.h, Game/Enemy/BulletMan/BulletManDirections.h).
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
        //Pair contains direction range. Value provides the corresponding Direction for the given pair range.
        using DirectionMap = std::unordered_map<std::pair<int, int>, Direction, PairHash>&;

        // Populates the map with default angle→direction ranges
        static void PopulateDirectionRanges(DirectionMap mapToFill);

        //DirectionMap's key pair represents minimum and maximum degree range. The value is the corresponding Direction for degree range.
        //In Short, take the map and calculated angle, and return the Found Direction from angle.
        static Direction GetDirectionFromAngle(const std::unordered_map<std::pair<int, int>, Direction, PairHash>& DirectionMap, float angle);

        //Which way `selfPosition` has to face to look at `targetPosition`. Used by the enemies to face the hero,
        //but it knows nothing about the hero - it takes two points
        static Direction GetDirectionToTarget(const ETG::Vector2f& targetPosition, const ETG::Vector2f& selfPosition);
    };
}
