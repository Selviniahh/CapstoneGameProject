#pragma once
#include "../Base/GunBase.h"

namespace ETG
{
    class Magnum : public GunBase
    {
    public:
        explicit Magnum(const sf::Vector2f& position);
        ~Magnum() override = default;
        void Initialize() override;

        BOOST_DESCRIBE_CLASS(Magnum, (GunBase), (), (), ())
    };

    class MagnumAnimComp : public BaseAnimComp<GunStateEnum>
    {
    public:
        MagnumAnimComp();
        void SetAnimations() override;
        bool IsAnimationFinished();
        BOOST_DESCRIBE_CLASS(MagnumAnimComp, (BaseAnimComp), (), (), ())
    };
}
