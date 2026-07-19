#pragma once
#include <cmath>
#include "../../Characters/Hero.h"
#include "../../../Engine/Core/GameObjectBase.h"
#include "../../../Engine/Managers/GameState.h"
#include "../../../Utils/Math.h"
#include "../../../Engine/Managers/AssetManager.h"

namespace ETG
{
    class Hero;
    class GunBase;
}

namespace ETG
{
    class ReloadText : public GameObjectBase
    {
    public:
        ReloadText();
        ~ReloadText() override = default;
        void Initialize() override;
        void Update() override;
        void Draw() override;
        void SetNeedsReload(const bool needsReload) { NeedsReload = needsReload; }

        // Method to link to a gun's events
        void LinkToGun(GunBase* gun);

    private:
        bool isVisible = true;
        float TextureYOffset = -20;
        Hero* Hero{nullptr};
        GunBase* CurrentGun{nullptr};
        bool NeedsReload = false;

        float BlinkInterval = 2.5f; //total time for fade cycle
        float blinkTimer{};

        // Callback for the ammo state changed event
        void OnAmmoStateChanged(bool isEmpty);

        BOOST_DESCRIBE_CLASS(ReloadText, (GameObjectBase), (), (), (isVisible,TextureYOffset, NeedsReload, BlinkInterval, blinkTimer))
    };

    inline ReloadText::ReloadText()
    {
        ReloadText::Initialize();
    }

    inline void ReloadText::Initialize()
    {
        Hero = GameState::GetInstance().GetHero();
        Texture = std::make_shared<ETG::Texture>();
        Texture->loadFromFile(AssetManager::Resolve("UI/ReloadText.png"));
        Origin.x = Texture->getSize().x / 2;
        Origin.y = Texture->getSize().y / 2;
        Scale = ETG::Vector2f{0.2f, 0.2f};
        Color = ETG::Color{255, 255, 255, 255};
        GameObjectBase::Initialize();
    }

    //We will relink the gun when we change the gun
    inline void ReloadText::LinkToGun(GunBase* gun)
    {
        if (gun)
        {
            // Register our callback with the gun's event
            CurrentGun = gun;
            CurrentGun->OnAmmoRunOut.Clear();
            CurrentGun->OnAmmoRunOut.AddListener([this](const bool isEmpty)
            {
                this->OnAmmoStateChanged(isEmpty);
            });
        }
    }

    inline void ReloadText::OnAmmoStateChanged(const bool isEmpty)
    {
        NeedsReload = isEmpty;
        if (!isEmpty)
        {
            // Reset states when we reload
            isVisible = true;
        }
    }

    inline void ReloadText::Update()
    {
        if (!NeedsReload) return;
        Position = Hero->GetPosition() + ETG::Vector2f{0, TextureYOffset};

        Color.a = Math::SinWaveLerp(25.f, 255.f, BlinkInterval, blinkTimer);
        GameObjectBase::Update();
    }

    inline void ReloadText::Draw()
    {
        if (!NeedsReload) return;

        if (isVisible)
        {
            GameObjectBase::Draw();
        }
    }
}
