#pragma once
#include <unordered_map>
#include "../../Core/ComponentBase.h"
#include "../../Utils/DirectionUtils.h"
#include "../../Core/GameObjectBase.h"

namespace ETG
{
    struct PairHash;
    enum class Direction;
    enum class HeroDashEnum;
    class Hero;

    class InputComponent : public ComponentBase
    {
    public:
        InputComponent();

        void Update(Hero& hero) const;

        static HeroDashEnum GetDashDirectionEnum();

        void PopulateSpecificWidgets() override;

    private:
        std::unordered_map<std::pair<int, int>, Direction, PairHash> DirectionMap{};
        void UpdateDirection(Hero& hero) const;

        BOOST_DESCRIBE_CLASS(InputComponent, (ComponentBase),
            (),
            (),
            ())
    };
}
