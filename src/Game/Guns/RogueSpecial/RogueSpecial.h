#pragma once
#include <boost/describe.hpp>
#include "../../../Engine/Core/Components/BaseAnimComp.h"
#include "../Base/GunBase.h"


namespace ETG
{
    class RogueSpecial : public GunBase
    {
    public:
        explicit RogueSpecial(const ETG::Vector2f& Position);
        ~RogueSpecial() override = default;

        void Initialize() override;
        void Update() override;

        BOOST_DESCRIBE_CLASS(RogueSpecial, (GunBase), (), (), ())
    };

    class RogueSpecialAnimComp : public BaseAnimComp<GunStateEnum>
    {
    public:
        RogueSpecialAnimComp();
        void SetAnimations() override;
        BOOST_DESCRIBE_CLASS(RogueSpecialAnimComp, (BaseAnimComp), (), (), ())
    };
}
