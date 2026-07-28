#pragma once
#include <unordered_map>
#include "../../../../Engine/Core/ComponentBase.h"
#include "../../../../Utils/DirectionUtils.h"
#include "../../../../Engine/Core/GameObjectBase.h"

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
        static void HandleDash(Hero& hero);
        void Update(Hero& hero) const;

    public:
        void PopulateSpecificWidgets() override;

    private:
        //NOTE: The angle -> Direction table used to be a member here, a copy of the one DirectionUtils already had
        //hard-wired into GetDirectionToTarget for the enemies. It lives in DirectionUtils now and this component
        //only reads it, so the hero and the enemies can no longer face the same angle differently
        void UpdateDirection(Hero& hero) const;
        void HandleGunSwitch(Hero& hero) const;
        mutable bool gunSwitchHandled = false; //Do it once

        BOOST_DESCRIBE_CLASS(InputComponent, (ComponentBase), (), (), ())
    };
}
