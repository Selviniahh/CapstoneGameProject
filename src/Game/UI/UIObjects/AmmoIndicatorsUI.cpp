#include "AmmoIndicatorsUI.h"
#include "AmmoBarUI.h"
#include "../../Guns/Base/GunBase.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Managers/SpriteBatch.h"

namespace ETG
{
    AmmoIndicatorsUI::AmmoIndicatorsUI()
    {
        AmmoIndicatorsUI::Initialize();
        GameObjectBase::Initialize();
    }

    void AmmoIndicatorsUI::Initialize()
    {
        // Base initialization happens here
    }

    void AmmoIndicatorsUI::SetBottomBar(AmmoBarUI* bottom)
    {
        bottomBar = bottom;

        // Update indicators if we already have a gun reference
        if (currentGun)
        {
            UpdateIndicators();
        }
    }

    void AmmoIndicatorsUI::SetTopBarPositionCallback(std::function<void(float)> callback)
    {
        topBarPositionCallback = callback;
    }

    void AmmoIndicatorsUI::Update()
    {
        GameObjectBase::Update();

        // Only update if we have all necessary references
        if (currentGun && bottomBar)
        {
            UpdateIndicators();
        }
    }

    void AmmoIndicatorsUI::UpdateIndicators()
    {
        // Clear previous indicators
        ammoIndicatorProps.clear();

        if (!currentGun || !currentGun->ProjTexture || !bottomBar) throw std::runtime_error("There's no current gun");

        // Get magazine info from gun
        const int magazineSize = currentGun->MagazineSize;
        const int currMagAmmoSize = currentGun->MagazineAmmo;

        if (magazineSize <= 0) return;

        // Get position for bottom bar
        const ETG::Vector2f bottomPos = bottomBar->GetPosition();

        // Calculate spacing based on projectile texture size
        const float ammoHeight = static_cast<float>(currentGun->ProjTexture->getSize().y);
        EachAmmoSpacing = ammoHeight * indicatorSpacing;

        // Start X position (centered with the bottom bar)
        const float startX = bottomPos.x;

        // Create indicators - start from bottom and move upward
        float currentY = bottomPos.y - EachAmmoSpacing; // Start just above the bottom bar
        float topMostY = currentY; // Track the topmost indicator position

        for (int i = 0; i < magazineSize; i++)
        {
            DrawProperties props;
            props.Position = {startX, currentY};
            props.Scale = {1.0f, 1.0f};
            props.Origin = {
                static_cast<float>(currentGun->ProjTexture->getSize().x) / 2.0f,
                static_cast<float>(currentGun->ProjTexture->getSize().y) / 2.0f
            };
            props.Rotation = 0.0f;
            props.Depth = 0.0f;
            props.Texture = currentGun->ProjTexture.get();

            // Apply visual styling based on ammo state
            if (i >= currMagAmmoSize)
            {
                // Used or empty ammo
                props.Color = ETG::Color(255, 255, 255, 128); // 50% alpha for empty slots
            }
            else
            {
                // Available ammo
                props.Color = ETG::Color::White;
            }

            ammoIndicatorProps.push_back(props);

            // Move up for next indicator
            currentY -= EachAmmoSpacing;
            topMostY = currentY; // Update topmost position
        }

        // After all indicators are positioned, calculate where the top bar should be
        const float topBarY = topMostY;

        // Notify the top bar's new position if callback is set
        if (topBarPositionCallback)
        {
            topBarPositionCallback(topBarY);
        }
    }

    void AmmoIndicatorsUI::Draw()
    {
        // Draw all ammo indicators
        for (const auto& props : ammoIndicatorProps)
        {
            SpriteBatch::Draw(props);
        }
    }
}
