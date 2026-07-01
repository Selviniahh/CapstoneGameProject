#pragma once
#include <memory>
#include "Platform/Platform.h"
#include "../Core/GameObjectBase.h"
#include "UIObjects/AmmoBarUI.h"
#include "UIObjects/AmmoCounter.h"
#include "UIObjects/AmmoIndicatorsUI.h"
#include "UIObjects/FrameLeftProgressBar.h"
#include "UIObjects/FrameBar.h"

namespace ETG
{
    class FrameLeftProgressBar;
    class GunBase;
    class Hero;
    class FrameBar;
    class ReloadSlider;

    class UserInterface : public GameObjectBase
    {
        // Draw properties
        // DrawProperties RightFrameDrawProps;
        DrawProperties gunDrawProps;

        // References
        GunBase* CurrentGun = nullptr;
        ActiveItemBase* CurrActiveItem = nullptr;
        Hero* hero = nullptr;

        // UI Components
        std::unique_ptr<FrameBar> RightGunFrame;
        std::unique_ptr<FrameBar> LeftActiveItemFrame;
        std::unique_ptr<AmmoBarUI> ammoBarBottom;
        std::unique_ptr<AmmoBarUI> ammoBarTop;
        std::unique_ptr<AmmoIndicatorsUI> ammoIndicators;
        std::unique_ptr<AmmoCounter> ammoCounter;
        std::unique_ptr<FrameLeftProgressBar> LeftProgressBar;  // ← Add this member


        // Configuration
        ETG::Vector2f RightFrameOffsetPerc{4, 3.5};
        ETG::Vector2f LeftFrameOffsetPerc{1, 3.5};

        //Ammo display at right UI
        float AmmoBarOffsetPercX = 2.f;
        float InitialAmmoBarOffsetY = 50.f;

        //Left passive item UI
        float LeftXOffsetPerItem = 10.f;
        float InitialLeftOffsetX = 35.f;
        float InitialLeftOffsetY = 18.f;

        //Get WindowSize - EngineUISize to calculate GameScreen
        ETG::Vector2f GameScreenSize;

    public:
        UserInterface();
        void InitializeFrameProperties();
        void InitializeAmmoBar();
        void Initialize() override;
        void UpdateGunUIProperties();
        void Update() override;
        void DrawEquippedPassiveItemsAtLeftUI() const;
        void Draw() override;

        BOOST_DESCRIBE_CLASS(UserInterface, (GameObjectBase),
                             (), (), (CurrentGun, RightFrameOffsetPerc, AmmoBarOffsetPercX, InitialAmmoBarOffsetY, GameScreenSize))
    };
}
