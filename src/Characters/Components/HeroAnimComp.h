#pragma once
#include <SFML/Graphics.hpp>
#include "../../Core/Components/BaseAnimComp.h"
#include "../../Characters/Hero.h"

namespace ETG
{
    class HeroAnimComp : public BaseAnimComp<HeroStateEnum>
    {
    public:
        HeroAnimComp();

        //Override
        void Update() override;
        void SetAnimations() override;
    
    private:
        Hero* HeroPtr = nullptr;

        BOOST_DESCRIBE_CLASS(HeroAnimComp, (BaseAnimComp), (HeroPtr), (), ())
    };
}
