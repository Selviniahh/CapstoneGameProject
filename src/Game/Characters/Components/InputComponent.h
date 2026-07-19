#pragma once
#include <unordered_map>
#include "../../../Engine/Core/ComponentBase.h"
#include "../../../Utils/DirectionUtils.h"
#include "../../../Engine/Core/GameObjectBase.h"

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
        static void HandleDash(const Hero& hero);
        void Update(Hero& hero) const;

    public:
        void PopulateSpecificWidgets() override;

    private:
        std::unordered_map<std::pair<int, int>, Direction, PairHash> DirectionMap{};
        void UpdateDirection(Hero& hero) const;
        void HandleGunSwitch(Hero& hero) const;
        mutable bool gunSwitchHandled = false; //Do it once

        BOOST_DESCRIBE_CLASS(InputComponent, (ComponentBase), (), (), ())
    };
}
