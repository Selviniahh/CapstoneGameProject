#include "ReloadSlider.h"
#include "../../Guns/Base/GunBase.h"
#include "../../Characters/Hero.h"
#include "../../Managers/SpriteBatch.h"
#include "../../Utils/Math.h"

ETG::ReloadSlider::ReloadSlider()
{
    SliderBar = std::make_shared<ETG::Texture>();
    SliderValue = std::make_shared<ETG::Texture>();
    ReloadSlider::Initialize();
}

void ETG::ReloadSlider::Initialize()
{
    Hero = GameState::GetInstance().GetHero();

    SliderBar->loadFromFile((std::filesystem::path(RESOURCE_PATH) / "UI" / "Slider.png").string());
    SliderValue->loadFromFile((std::filesystem::path(RESOURCE_PATH) / "UI" / "SliderValue.png").string());

    SliderBarPros.Texture = SliderBar.get();
    SliderValProps.Texture = SliderValue.get();

    // Set center origin
    SliderBarPros.Origin = {static_cast<float>(SliderBarPros.Texture->getSize().x) / 2, static_cast<float>(SliderBarPros.Texture->getSize().y) / 2};
    SliderValProps.Origin = {static_cast<float>(SliderValProps.Texture->getSize().x) / 2, static_cast<float>(SliderValProps.Texture->getSize().y) / 2};

    //1/4 scale looks best 
    SliderBarPros.Scale = {0.25f, 0.25f};
    SliderValProps.Scale = {0.25f, 0.25f};

    GameObjectBase::Initialize();
}

void ETG::ReloadSlider::Update()
{
    // Update position regardless of state
    SliderBarPros.Position = Hero->GetPosition() + ETG::Vector2f{0, -BarOffsetY};

    if (!IsAnimating) return;

    //Continue the animation if it's active
    StartAnimation();

    GameObjectBase::Update();
}

void ETG::ReloadSlider::Draw()
{
    // Only draw when animating
    if (!IsAnimating) return;

    SpriteBatch::Draw(SliderBarPros);
    SpriteBatch::Draw(SliderValProps);
}

void ETG::ReloadSlider::OnReloadStart(const bool isReloading)
{
    if (isReloading)
    {
        IsAnimating = true;
        reloadTimer = 0.0f;
    }
}

void ETG::ReloadSlider::OnReloadComplete()
{
    IsAnimating = false;
    reloadTimer = 0.0f;
}

void ETG::ReloadSlider::StartAnimation()
{
    // Set initial positions before starting the animation
    SliderBarPros.Position = Hero->GetPosition() + ETG::Vector2f{0, -BarOffsetY};
    SliderValProps.Position = SliderBarPros.Position;

    // Calculate reload progress
    if (Gun && Gun->IsReloading)
    {
        // Update the progress bar
        auto [TopLeft, TopRight, BottomLeft, BottomRight] // Four corners
            = Math::CalculateFourCorner(SliderBarPros.Position, ETG::Vector2f(SliderBarPros.Texture->getSize()), SliderBarPros.Origin, SliderBarPros.Scale);

        const ETG::Vector2f LeftMidPos = ETG::Vector2f{TopLeft.x / 2, TopLeft.y / 2} + ETG::Vector2f{BottomLeft.x / 2, BottomLeft.y / 2};
        const ETG::Vector2f RightMidPos = ETG::Vector2f{TopRight.x / 2, TopRight.y / 2} + ETG::Vector2f{BottomRight.x / 2, BottomRight.y / 2};
        SliderValProps.Position.x = Math::IntervalLerp(LeftMidPos.x, RightMidPos.x, Gun->ReloadTime, reloadTimer);
        reloadTimer += Globals::FrameTick;

        // 1. Primary method: Position-based check with tolerance
        bool positionReached = std::abs(SliderValProps.Position.x - RightMidPos.x) <= PositionTolerance;
        
        // 2. Safety method: Timer-based check
        bool timeComplete = reloadTimer >= Gun->ReloadTime;
        
        // 3. Additional safety: If reload time is zero or negative
        bool invalidReloadTime = Gun->ReloadTime <= 0.0f;

        // Complete the animation if ANY condition is met
        if (positionReached || timeComplete || invalidReloadTime)
        {
            FinishAnimation();
        }
    }
    else
    {
        // Fix: If gun is no longer in reloading state but we're still animating
        // (handles edge case where gun state changes externally)
        FinishAnimation();
    }
}

void ETG::ReloadSlider::FinishAnimation()
{
        // Animation complete - set gun state
        Gun->IsReloading = false;
        Gun->MaxAmmo -= Gun->MagazineSize - Gun->MagazineAmmo;
        Gun->MagazineAmmo = Gun->MagazineSize;
        Gun->CurrentGunState = GunStateEnum::Idle;

        // Reset our state
        OnReloadComplete();
}

void ETG::ReloadSlider::LinkToGun(GunBase* gun)
{
    if (!gun) throw std::runtime_error("Gun not found");
    Gun = gun;

    // Register for reload start events
    gun->OnReloadInvoke.AddListener([this](const bool isReloading)
    {
        this->OnReloadStart(isReloading);
    });
}
