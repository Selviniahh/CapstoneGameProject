#include "../../../Engine/Managers/Time.h"
#include "ReloadSlider.h"
#include "../../Guns/Base/GunBase.h"
#include "../../Characters/Hero/Hero.h"
#include "../../../Engine/Managers/SpriteBatch.h"
#include "../../../Utils/Math.h"
#include "../../../Engine/Managers/AssetManager.h"

ETG::ReloadSlider::ReloadSlider()
{
    SliderBar = std::make_shared<ETG::Texture>();
    SliderValue = std::make_shared<ETG::Texture>();
    ReloadSlider::Initialize();
}

void ETG::ReloadSlider::Initialize()
{
    Hero = Hero::Get();

    SliderBar->loadFromFile(AssetManager::Resolve("UI/Slider.png"));
    SliderValue->loadFromFile(AssetManager::Resolve("UI/SliderValue.png"));

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

    // Bar hero'nun UI'ıdır: konumu her frame Hero'nun tepesine yazılır. Enemy'ler de artık kendi silahlarını
    // reload ettiği için (BulletMan::UpdateShooting), bu kontrol olmasaydı enemy'nin reload'u hero'nun başının
    // üstünde sahipsiz bir bar olarak belirirdi. Update tarafı gate'lenmez - reload'u bitirip ammo'yu dolduran
    // FinishAnimation oradan çalışır ve enemy için de çalışmak zorundadır.
    if (!Gun || !Gun->Owner || !Gun->Owner->IsA<class Hero>()) return;

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
        //NOTE: .Get() rather than letting the Stat convert, because IntervalLerp deduces one type across all three
        //of its value parameters and a Stat next to two floats gives it nothing to deduce
        SliderValProps.Position.x = Math::IntervalLerp(LeftMidPos.x, RightMidPos.x, Gun->ReloadTime.Get(), reloadTimer);
        reloadTimer += Time::FrameTick;

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
        Gun->MaxAmmo -= Gun->MagazineSize.GetInt() - Gun->MagazineAmmo;
        Gun->MagazineAmmo = Gun->MagazineSize.GetInt();
        Gun->CurrentGunState = GunStateEnum::Idle;

        // Reset our state
        OnReloadComplete();
}

void ETG::ReloadSlider::LinkToGun(GunBase* gun)
{
    if (!gun) throw std::runtime_error("Gun not found");
    Gun = gun;

    //Cleared first, the way ReloadText::LinkToGun does it. This is a re-link point, not a construction one: it
    //runs again whenever the same gun is linked a second time, and without this each pass left another live
    //listener on that gun's OnReloadInvoke. Clearing is safe because this slider is the only thing listening
    gun->OnReloadInvoke.Clear();

    // Register for reload start events
    gun->OnReloadInvoke.AddListener([this](const bool isReloading)
    {
        this->OnReloadStart(isReloading);
    });
}
