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
        void Draw() override;
        void Reload() override;

        BOOST_DESCRIBE_CLASS(RogueSpecial, (GunBase), (), (), ())

    protected:
        //Reload VFX. `class` is required on both: GunBase's own MuzzleFlash *member*
        //hides the type name in this scope.
        std::unique_ptr<class MuzzleFlash> ReloadFlash; //green flash off the barrel tip
        std::unique_ptr<class MuzzleFlash> ReloadSmoke; //smoke venting from underneath
    };

    class RogueSpecialAnimComp : public BaseAnimComp<GunStateEnum>
    {
    public:
        RogueSpecialAnimComp();
        void SetAnimations() override;
        BOOST_DESCRIBE_CLASS(RogueSpecialAnimComp, (BaseAnimComp), (), (), ())
    };
}
