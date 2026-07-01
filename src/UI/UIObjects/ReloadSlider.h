#pragma once
#include "../../Core/GameObjectBase.h"

namespace ETG
{
    class GunBase;
    class Hero;
    
    class ReloadSlider : public GameObjectBase
    {
    public:
        ReloadSlider();
        ~ReloadSlider() override = default;

        void Initialize() override;
        void Draw() override;
        void Update() override;

        void LinkToGun(GunBase* gun);

    private:
        void StartAnimation();
        void FinishAnimation();
        
        // Handle gun events
        void OnReloadStart(bool isReloading);
        void OnReloadComplete();
        
        std::shared_ptr<ETG::Texture> SliderBar;
        std::shared_ptr<ETG::Texture> SliderValue;
        float reloadTimer = 0.0f;

        DrawProperties SliderBarPros;
        DrawProperties SliderValProps;

        float BarOffsetY = 20.0f;
        float PositionTolerance = 0.10f;

        Hero* Hero = nullptr;
        GunBase* Gun = nullptr;
        bool IsAnimating = false;

        BOOST_DESCRIBE_CLASS(ReloadSlider, (GameObjectBase), (), (), (SliderBar, SliderValue, reloadTimer, BarOffsetY, PositionTolerance, IsAnimating))
    };    
}
