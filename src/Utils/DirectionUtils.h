#pragma once
#include <functional>
#include <string>
#include "Platform/Platform.h"
#include "../../src/Managers/Enum/StateEnums.h"

//NOTE: For now this class hosts basic enum operations for both hero and enemy. Later on I will decide if it's worth to separate this class
namespace ETG
{
    enum class Direction;
    class Hero;

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
        static Direction GetHeroDirectionFromAngle(const std::unordered_map<std::pair<int, int>, Direction, PairHash>& DirectionMap, float angle);
        static BulletManShootingEnum GetBulletManShootingEnum(Direction currDir);
        static BulletManHitEnum GetBulletManHitEnum(Direction currDir);
        static BulletManDeathEnum GetBulletManDeathEnum(Direction currDir);

        static Direction LastDashDirection;
        static const Direction& GetDirectionFromDash() {return LastDashDirection;} //This will run instead of above GetHeroDirectionFromAngle if state is Dash

        //Hero enum related 
        static HeroIdleEnum GetHeroIdleDirectionEnum(Direction currDir);
        static HeroRunEnum GetHeroRunEnum(Direction currDir);
        static HeroDashEnum GetDashDirectionEnum();
        static ETG::Vector2f GetDashDirectionVector();

        //Enemy BulletMan related 
        static Direction GetDirectionToHero(const Hero* Hero, ETG::Vector2f SelfPosition);

        static BulletManIdleEnum GetBulletManIdleEnum(Direction currDir);
        static BulletManRunEnum GetBulletManRunEnum(Direction currDir);
    };
    
}
