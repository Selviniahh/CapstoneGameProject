#pragma once
#include "../../../Engine/Core/GameObjectBase.h"
#include "../../../Engine/Platform/Platform.h"

namespace ETG
{
    class ActiveItemBase;
    
    class FrameLeftProgressBar : public GameObjectBase
    {
    public:
        FrameLeftProgressBar();
        ~FrameLeftProgressBar() override = default;
        
        void Initialize() override;
        void Draw() override;
        void Update() override;
        
        // Set the active item to display progress for
        void SetActiveItem(ActiveItemBase* item) { activeItem = item; }
        
        // Get background/foreground colors for customization
        void SetProgressColor(const ETG::Color& color) { progressColor = color; }
        
    private:
        ActiveItemBase* activeItem = nullptr;
        ETG::Color progressColor = ETG::Color(255, 255, 0); // Default yellow
        
        // Progress bar elements
        ETG::RectangleShape progressRect;
        float maxWidth = 0.0f;
        float maxHeight = 0.0f;

        float CurrProgressY{}; //This will change in tick
        float TotalProgressLength{}; //ProgTopCenter.y - ProgBottomCenter.y For now it's always 120

        ETG::Vector2f ProgTopCenter; //Progress bar's top center position
        ETG::Vector2f ProgBottomCenter; //Progress bar's bottom center position
    };
}