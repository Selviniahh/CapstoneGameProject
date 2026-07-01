#pragma once
#include "../../Core/GameObjectBase.h"
#include "Platform/Platform.h"
#include <memory>
#include <vector>
#include <functional>

namespace ETG
{
    class GunBase;
    class AmmoBarUI;

    class AmmoIndicatorsUI : public GameObjectBase
    {
    private:
        // References to ammo bars for positioning
        AmmoBarUI* bottomBar = nullptr;

        // References for game state
        GunBase* currentGun = nullptr;

        // Ammo indicator properties
        std::vector<DrawProperties> ammoIndicatorProps;

        // Spacing configuration
        float indicatorSpacing = 3.5f; // Multiplier for spacing (1.2 = 20% extra space between indicators) leave it 3.5 for now

        // Callback for positioning the top bar
        std::function<void(float)> topBarPositionCallback;

    public:
        float EachAmmoSpacing;

        AmmoIndicatorsUI();
        ~AmmoIndicatorsUI() override = default;

        void Initialize() override;
        void Update() override;
        void Draw() override;

        // Set reference to bottom bar
        void SetBottomBar(AmmoBarUI* bottom);

        // Set gun reference
        void SetGun(GunBase* gun) { currentGun = gun; };

        // Register callback for top bar positioning
        void SetTopBarPositionCallback(std::function<void(float)> callback);

        // Update indicator positions and appearance based on current gun state
        void UpdateIndicators();

        BOOST_DESCRIBE_CLASS(AmmoIndicatorsUI, (GameObjectBase), (), (), (currentGun, indicatorSpacing))
    };
}
