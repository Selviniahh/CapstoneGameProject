#pragma once
#include <unordered_map>
#include "../../Utils/DirectionUtils.h"
#include "../../Core/GameObject.h"
namespace ETG
{
    struct PairHash;
    enum class Direction;
    enum class DashEnum;
    class Hero;
    
    class InputComponent : public GameObject
    {
    public:
        InputComponent();

        void Update(Hero& hero) const;

        static DashEnum GetDashDirectionEnum();

    private:
        std::unordered_map<std::pair<int, int>, Direction, PairHash> DirectionMap{};
        void UpdateDirection(Hero& hero) const;
    };
}
