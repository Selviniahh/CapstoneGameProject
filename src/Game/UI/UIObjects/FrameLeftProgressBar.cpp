#include <filesystem>
#include "FrameLeftProgressBar.h"
#include "../../Items/Active/ActiveItemBase.h"
#include "../../../Engine/Managers/Globals.h"
#include "../../Managers/GameState.h"
#include "../../../Engine/Managers/SpriteBatch.h"
#include "../../../Utils/Math.h"
#include "../../../Engine/Managers/AssetManager.h"

ETG::FrameLeftProgressBar::FrameLeftProgressBar()
{
    Texture = std::make_shared<ETG::Texture>();
    if (!Texture->loadFromFile(AssetManager::Resolve("UI/FrameLeftProgressBar.png")))
        throw std::runtime_error("Failed to load FrameLeftProgressBar texture");

    // Set up the progress rectangle
    progressRect.setFillColor(progressColor);

    // Set dimensions based on texture
    if (Texture)
    {
        maxWidth = static_cast<float>(Texture->getSize().x - 4); // Slight margin
        maxHeight = static_cast<float>(Texture->getSize().y - 4);
        progressRect.setSize({maxWidth, maxHeight}); // Start with full height
    }

    FrameLeftProgressBar::Initialize();
}

void ETG::FrameLeftProgressBar::Initialize()
{
    GameObjectBase::Initialize();

    Origin = {Texture->getSize().x / 2.f, Texture->getSize().y / 2.f};
}

void ETG::FrameLeftProgressBar::Update()
{
    GameObjectBase::Update();

    //Calculate Top and Bottom center. Bcs progressRect is not GameObjectBase derived, we have to calculate center origin ourselves
    ProgTopCenter = {Position.x, Position.y - maxHeight / 2};
    ProgBottomCenter = {Position.x, Position.y + maxHeight / 2};
    TotalProgressLength = std::abs(ProgTopCenter.y - ProgBottomCenter.y);

    if (activeItem)
        
        //NOTE: Consuming state: Progress decreases from top (100%) to bottom (0%)
        if (activeItem->ActiveItemState == ActiveItemState::Consuming)
        {
            //Set active item color back to normal 
            activeItem->SetColor(ETG::Color{94,94,94});
            CurrProgressY = Math::IntervalLerp(TotalProgressLength, 0.f, activeItem->TotalConsumeTime, activeItem->ConsumeTimer);
            progressRect.setFillColor(ETG::Color::White);
            progressRect.setPosition(ProgBottomCenter);
            progressRect.setSize({maxWidth, -CurrProgressY});
            progressRect.setOrigin(progressRect.getSize().x / 2, 0);
            IsVisible = true;
        }

        //NOTE: Cooldown is over. Item is ready to be consumed. Set color back to normal.
        else if (activeItem->ActiveItemState == ActiveItemState::Ready)
        {
            //If the item is ready, do not draw the frame left progress bar UI
            if (activeItem->ActiveItemState == ActiveItemState::Ready) IsVisible = false;
            activeItem->SetColor(ETG::Color::White);
        }

        //NOTE: COOLDOWN STATE: Progress increases from bottom (0%) to top (100%)
        else if (activeItem->ActiveItemState == ActiveItemState::Cooldown)
        {
            //During cooldown, both item and progress bar needs to be gray
            activeItem->SetColor(ETG::Color{94,94,94});
            CurrProgressY = Math::IntervalLerp(0.f, TotalProgressLength, activeItem->TotalCooldownTime, activeItem->CoolDownTimer);
            progressRect.setPosition(ProgBottomCenter);
            progressRect.setSize({maxWidth, -CurrProgressY});
            IsVisible = true;
            
        }
}


void ETG::FrameLeftProgressBar::Draw()
{
    // Draw the frame background first
    GameObjectBase::Draw();

    // Draw rectangle with higher depth to ensure it appears on top
    ETG::RenderWindow* window = GameState::GetInstance().GetRenderWindow();
    if (window && activeItem && activeItem->ActiveItemState != ActiveItemState::Ready)
    {
        // Fallback direct drawing in case SpriteBatch method fails
        window->draw(progressRect);
        
        // Try SpriteBatch method as well
        // SpriteBatch::AddDebugCircle(ProgTopCenter, 15);
        // SpriteBatch::AddDebugCircle(ProgBottomCenter, 15);
    }
}
