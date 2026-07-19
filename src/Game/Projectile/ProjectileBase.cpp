#include "../../Engine/Managers/Time.h"
#include "ProjectileBase.h"
#include <valarray>
#include "../Characters/Hero.h"
#include "../../Engine/Core/Factory.h"
#include "../../Engine/Core/Components/CollisionComponent.h"
#include "../../Engine/Core/Components/TimerComponent.h"
#include "../Enemy/EnemyBase.h"
#include "../../Engine/Managers/RenderContext.h"
#include "../../Engine/Managers/SpriteBatch.h"

ETG::ProjectileBase::~ProjectileBase() = default;

ETG::ProjectileBase::ProjectileBase(const ETG::Texture& texture, const ETG::Vector2f spawnPos, const ETG::Vector2f velocity, const float range, const float rotation, const float damage, const float force)
{
    Position = spawnPos;
    ProjVelocity = velocity;
    Range = range;
    Texture = std::make_shared<ETG::Texture>(texture);
    Rotation = rotation;
    Damage = damage;
    Force = force;

    //NOTE: For now only hero's projectiles will be destroyed when collided with enemies. The projectile will 
    TimerComp = ETG::CreateGameObjectAttached<TimerComponent>(this, 0.1f);

    CollisionComp = ETG::CreateGameObjectAttached<CollisionComponent>(this);
    CollisionComp->CollisionRadius = 1.0f;
    CollisionComp->CollisionVisualizationColor = ETG::Color::Blue;
    CollisionComp->SetCollisionEnabled(true);

    CollisionComp->OnCollisionEnter.AddListener([this](const CollisionEventData& eventData)
    {
        // Check if we collided with enemy. 
        const auto heroObj = dynamic_cast<Hero*>(this->Owner->Owner);
        const auto* enemyObj = dynamic_cast<EnemyBase*>(eventData.Other);
        
        if (heroObj && enemyObj)
        {
            // std::cout << "we need to play impact animation here for " << this->Owner->Owner->GetObjectName() << " 's projectile and then destroy but nevermind " << std::endl;

            //This is Hero's projectile that collided with an enemy
            enemyObj->CollisionComp->OnCollisionEnter.Broadcast(CollisionEventData{this, this, eventData.OtherComp,eventData.ImpactPoint}); 
            this->MarkForDestroy();
        }
    });

    //Should we check if hero's projectile collided with enemy's projectile and then play a cool explosion VFX and then remove both projectiles?

    
}

void ETG::ProjectileBase::Initialize()
{
}

void ETG::ProjectileBase::Update()
{
    if (PendingDestroy) return;
    TimerComp->Update();
    CollisionComp->Update();

    const ETG::Vector2f movement = Time::FrameTick * ProjVelocity;
    Position += movement;

    //Calculate distance traveled so far
    const float frameDistance = std::sqrt(movement.x * movement.x + movement.y * movement.y);
    DistanceTraveled += frameDistance;

    //If projectile has exceeded it's range destroy
    if (DistanceTraveled >= Range)
    {
        MarkForDestroy();
        return;
    }

    GameObjectBase::Update();
}

void ETG::ProjectileBase::Draw()
{
    IsVisible = true;
    if (CollisionComp) CollisionComp->Visualize(*ETG::RenderContext::Window);
    auto& DrawableProps = GetDrawProperties();
    ETG::Sprite frame;
    frame.setTexture(*Texture);
    frame.setOrigin(frame.getTexture()->getSize().x / 2, frame.getTexture()->getSize().y / 2);
    frame.setPosition(DrawableProps.Position);
    frame.setRotation(DrawableProps.Rotation);
    ETG::GlobSpriteBatch.Draw(frame, 0);
}
