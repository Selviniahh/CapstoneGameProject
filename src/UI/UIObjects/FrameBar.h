#pragma once
#include "../../Core/GameObjectBase.h"

namespace ETG
{
    class ActiveItemBase;
    class GunBase;

    enum class BarType
    {
        GunBar,
        ActiveItemBar
    };

    class FrameBar : public GameObjectBase
    {
    public:
        explicit FrameBar(const std::string& texturePath, BarType type = BarType::GunBar);
        ~FrameBar() override = default;
        void Initialize() override;
        void SetDrawPropsOrientation();
        void Draw() override;
        void Update() override;

        // Set content to display
        void SetGun(GunBase* gun);
        void SetActiveItem(ActiveItemBase* item);

        // Getters
        BarType GetBarType() const { return barType; }

    private:
        BarType barType;
        GunBase* gunContent = nullptr;
        ActiveItemBase* itemContent = nullptr;
        DrawProperties contentDrawProps{};
        
        std::shared_ptr<ETG::Texture> FullFrameTexture;
        std::shared_ptr<ETG::Texture> FrameWithProgBarTexture;
        float contentScale = 3.0f; // Default scale factor for contents
    };
}
