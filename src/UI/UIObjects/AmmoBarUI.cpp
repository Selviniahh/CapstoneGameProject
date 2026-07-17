#include "AmmoBarUI.h"
#include "../../Guns/Base/GunBase.h"
#include "../../Managers/AssetManager.h"
#include "../../Managers/Globals.h"
#include "../../Managers/SpriteBatch.h"

namespace ETG
{
    AmmoBarUI::AmmoBarUI()
    {
        AmmoBarUI::Initialize();
        GameObjectBase::Initialize();
    }

    void AmmoBarUI::Initialize()
    {
        // Load textures
        ammoBarTexture = std::make_shared<ETG::Texture>();

        if (!ammoBarTexture->loadFromFile(AssetManager::Resolve("UI/AmmoBarUI.png")))
            throw std::runtime_error("Failed to load AmmoBarUI.png");

        // Set up initial draw properties
        const ETG::Vector2u barSize = ammoBarTexture->getSize();

        // Set up ammo bar draw properties
        Texture = ammoBarTexture;
        Origin = {
            static_cast<float>(barSize.x) / 2.0f,
            static_cast<float>(barSize.y) / 2.0f
        };
    }

    void AmmoBarUI::Update()
    {
        GameObjectBase::Update();
    }

    void AmmoBarUI::FlipTexture(const bool horizontally, const bool vertically)
    {
        float scaleX = horizontally ? -1.0f : 1.0f;
        float scaleY = vertically ? -1.0f : 1.0f;
    
        // Set base scale
        this->Scale = {scaleX, scaleY};
    }

    void AmmoBarUI::Draw()
    {
        // Draw ammo bar
        SpriteBatch::Draw(GetDrawProperties());
    }
}
