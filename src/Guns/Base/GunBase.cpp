#include <complex>
#include <filesystem>
#include "GunBase.h"
#include <numbers>
#include "../../Projectile/ProjectileBase.h"
#include "../../Managers/Globals.h"
#include "../../Managers/SpriteBatch.h"
#include "../../Core/Factory.h"

namespace ETG
{
    ETG::GunBase::GunBase(const sf::Vector2f Position, const float pressTime, const float velocity, const float maxProjectileRange, const float timerForVelocity)
        : pressTime(pressTime), velocity(velocity), maxProjectileRange(maxProjectileRange), timerForVelocity(timerForVelocity)
    {
        // Initialize common position and textures
        this->Position = Position;
        
        if (!Texture) Texture = std::make_shared<sf::Texture>();
        if (!ProjTexture) ProjTexture = std::make_shared<sf::Texture>();
        if (!Texture) Texture = std::make_shared<sf::Texture>();
        if (!ArrowTex) ArrowTex = std::make_shared<sf::Texture>();
    }

    GunBase::~GunBase()
    {
        for (auto& proj : projectiles)
            proj.reset();
    }

    void GunBase::Initialize()
    {
        //DEPRECATE: Remove this after next commit
        // this->Origin = {
        //     static_cast<float>(Texture->getSize().x / 2),
        //     static_cast<float>(Texture->getSize().y / 2)
        // };

        //The origin manually needs to be given because when gun rotating, it has to rotate around the attachment point which is the handle point of the gun. 
        this->Origin += OriginOffset;

        // Load the arrow texture (common for all guns).
        const std::filesystem::path arrowPath = std::filesystem::path(RESOURCE_PATH) / "Projectiles" / "Arrow.png";
        if (!ArrowTex->loadFromFile(arrowPath.generic_string()))
            throw std::runtime_error(arrowPath.generic_string() + " not found");

        arrowOrigin = {
            static_cast<float>(ArrowTex->getSize().x / 2),
            static_cast<float>(ArrowTex->getSize().y / 2)
        };
        arrowOrigin += arrowOriginOffset;

        // Muzzle flash animation should be set up by the derived class.
        muzzleFlashAnim.Active = false;
        this->Depth = 2;
    }

    void GunBase::Update()
    {
        GameObjectBase::Update();

        timerForVelocity += Globals::FrameTick;

        // If the shoot animation finished, revert to idle.
        if (AnimationComp->CurrentState == GunStateEnum::Shoot &&
            AnimationComp->AnimManagerDict[AnimationComp->CurrentState].IsAnimationFinished())
        {
            CurrentGunState = GunStateEnum::Idle;
        }

        // Update arrow position.
        arrowPos = this->Position + RotateVector(arrowOffset);

        // Update gun animation.
        AnimationComp->Update(CurrentGunState, CurrentGunState);
        Texture = AnimationComp->CurrentTex;

        // Update muzzle flash position and animation.
        MuzzleFlashPos = arrowPos + RotateVector(MuzzleFlashOffset);
        muzzleFlashAnim.Update();
        if (muzzleFlashAnim.Active && muzzleFlashAnim.IsAnimationFinished())
            muzzleFlashAnim.Active = false;

        // Update projectiles.
        for (const auto& proj : projectiles)
        {
            proj->Update();
        }
    }

    void GunBase::Draw()
    {
        GameObjectBase::Draw();
        
        // Draw the gun.
        const auto& DrawProps = this->GetDrawProperties();
        AnimationComp->Draw(DrawProps.Position, DrawProps.Origin, DrawProps.Scale, DrawProps.Rotation, DrawProps.Depth);
        Globals::DrawSinglePixelAtLoc(DrawProps.Position, {1, 1}, DrawProps.Rotation);

        // Draw the arrow representation.
        SpriteBatch::SimpleDraw(ArrowTex, arrowPos, DrawProps.Rotation, arrowOrigin);
        Globals::DrawSinglePixelAtLoc(arrowPos, {1, 1}, DrawProps.Rotation);

        // Draw projectiles.
        for (const auto& proj : projectiles)
        {
            proj->Draw();
        }

        // Draw the muzzle flash.
        //NOTE: Muzzle flash did not constructed as drawable object yet. So all the properties are arbitrarily created.
        muzzleFlashAnim.Draw(muzzleFlashAnim.Texture, MuzzleFlashPos, sf::Color::White, DrawProps.Rotation, muzzleFlashAnim.Origin, {1, 1}, DrawProps.Depth);
    }

    void GunBase::Shoot()
    {
        if (timerForVelocity >= pressTime)
        {
            //Set muzzleFlashAnim Active to true. Once the animation is drawn, it will be set back to false. 
            muzzleFlashAnim.Active = true;

            //Set animation to Shoot
            CurrentGunState = GunStateEnum::Shoot;
            timerForVelocity = 0;

            // Restart shoot animation.
            AnimationComp->AnimManagerDict[GunStateEnum::Shoot].AnimationDict[GunStateEnum::Shoot].Restart();

            // Calculate projectile velocity.
            const sf::Vector2f spawnPos = arrowPos;
            const float angle = Rotation;
            const float rad = angle * std::numbers::pi / 180.f;
            const sf::Vector2f direction(std::cos(rad), std::sin(rad));
            const sf::Vector2f projVelocity = direction * velocity;

            // Spawn projectile.
            // Fixed: Pass the texture directly if CreateGameObjectDefault expects sf::Texture
            std::unique_ptr<ProjectileBase> proj = ETG::CreateGameObjectDefault<ProjectileBase>(*ProjTexture, spawnPos, projVelocity, maxProjectileRange, Rotation);
            proj.get()->Update(); //Necessary to set initial position
            projectiles.push_back(std::move(proj));

            // Restart muzzle flash animation.
            muzzleFlashAnim.Restart();
        }
    }

    sf::Vector2f GunBase::RotateVector(const sf::Vector2f& offset) const
    {
        const float angleRad = Rotation * (std::numbers::pi / 180.f);
        sf::Vector2f scaledOffset(offset.x * this->Scale.x, offset.y * this->Scale.y);

        return {
            scaledOffset.x * std::cos(angleRad) - scaledOffset.y * std::sin(angleRad),
            scaledOffset.x * std::sin(angleRad) + scaledOffset.y * std::cos(angleRad)
        };
    }
}
