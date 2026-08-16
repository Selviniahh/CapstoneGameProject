#include "../../Engine/Managers/Time.h"
#include "ProjectileBase.h"
#include <valarray>
#include "../Characters/Hero/Hero.h"
#include "../../Engine/Core/Factory.h"
#include "../../Engine/Core/Components/CollisionComponent.h"
#include "../../Engine/Core/Components/TimerComponent.h"
#include "../Characters/Enemy/EnemyBase.h"
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
    CollisionComp->Name = "Projectile";

    //The two things a bullet can hit. Leaving Projectile out of the mask is where most of the saving is: a screen
    //full of bullets used to test every bullet against every other one, and none of those pairs ever mattered
    CollisionComp->Layer = CollisionLayer::Projectile;
    CollisionComp->Mask = CollisionLayer::Hero | CollisionLayer::Enemy;

    CollisionComp->SetCollisionEnabled(true);

    //Last statement, and the constructor's alone: see GameObjectBase::BindEvents
    ProjectileBase::BindEvents();
}

void ETG::ProjectileBase::BindEvents()
{
    CollisionComp->OnCollisionEnter.AddListener([this](const CollisionEventData& eventData)
    {
        // Check if we collided with enemy.
        const auto* heroObj = dynamic_cast<Hero*>(this->Owner->Owner);
        const auto* enemyObj = dynamic_cast<EnemyBase*>(eventData.Other);

        if (heroObj && enemyObj)
        {
            //Damage is the enemy's own listener to run, and it is already running: one overlap is detected once
            //and both sides are handed their own event out of it (CollisionSystem::DetectContacts). This used to
            //broadcast the enemy's OnCollisionEnter by hand, from right here, because back when every collider ran
            //its own pass inside its owner's Update there was no saying whether the enemy would find this bullet
            //itself, or on which frame. It always did find it - so the hand-made event was a second one on top,
            //and EnemyBase's listener spent one shot's damage twice. It also arrived without the "is this contact
            //new?" test that Enter is gated on, since Broadcast is the delegate, not the pass that decides
            //
            //Mermi burada yalnızca kendi işini yapar: durur ve iki dairenin değdiği noktada patlar. Yok etme işini
            //BeginImpact üstlenir: animation varsa bittiğinde, yoksa hemen.
            this->BeginImpact(eventData.ImpactPoint);
        }
    });

    //Should we check if hero's projectile collided with enemy's projectile and then play a cool explosion VFX and then remove both projectiles?
}

void ETG::ProjectileBase::Initialize()
{
}

void ETG::ProjectileBase::SetImpactAnimation(const Animation& impactAnim)
{
    ImpactAnim = impactAnim;

    // Bir kez oynayıp son frame'inde durur; IsFinished'ın "bitti" demesi yalnızca bu hâlde dürüsttür ve
    // projectile'ın yok edileceği an odur. Silah bunu unutsa bile burada garanti altına alınır.
    ImpactAnim.Loops = false;
    ImpactAnim.Restart();

    HasImpactAnim = true;
}

void ETG::ProjectileBase::BeginImpact(const ETG::Vector2f& impactPoint)
{
    // Impact animation'ı olmayan silahın mermisi eskisi gibi o frame kaybolur
    if (!HasImpactAnim)
    {
        MarkForDestroy();
        return;
    }

    // Aynı frame'de iki collision gelebilir; ilki kazanır, aksi hâlde animation kendini baştan başlatırdı
    if (Impacting) return;

    Impacting = true;
    ImpactPos = impactPoint;

    // Artık ne uçar ne bir şeye çarpar. Collision kapatılmazsa duran mermi aynı enemy'ye her frame yeniden
    // vurur ve tek bir shot, animation süresi boyunca damage yağdırırdı.
    ProjVelocity = {};
    CollisionComp->SetCollisionEnabled(false);

    ImpactAnim.Restart();

    // İlk frame'in rect'i şimdi hesaplanır: çarpışma bu frame'in Update'i içinde olur ve arkasından gelen Draw,
    // Animation::Update henüz CurrRect'i doldurmamışken çalışırdı.
    ImpactAnim.Update();
}

void ETG::ProjectileBase::Update()
{
    if (PendingDestroy) return;

    // Çarpma başladıysa geriye yalnızca VFX kalmıştır: mermi hareket etmez, animation bittiğinde object gider.
    // Collision'ı burada atlamak yetmez, çünkü taramayı artık CollisionSystem yapıyor ve o bu return'ü görmez;
    // aranmamasını sağlayan şey BeginImpact'in collision'ı kapatması.
    if (Impacting)
    {
        ImpactAnim.Update();
        if (ImpactAnim.IsFinished()) MarkForDestroy();
        return;
    }

    TimerComp->Update();

    const ETG::Vector2f movement = Time::FrameTick * ProjVelocity;
    Position += movement;

    //Calculate distance traveled so far
    const float frameDistance = std::sqrt(movement.x * movement.x + movement.y * movement.y);
    DistanceTraveled += frameDistance;

    //If projectile has exceeded it's range play the impact where it ran out and destroy afterwards
    if (DistanceTraveled >= Range)
    {
        BeginImpact(Position);
        return;
    }

    GameObjectBase::Update();
}

void ETG::ProjectileBase::Draw()
{
    IsVisible = true;
    
    if (CollisionComp) 
        CollisionComp->Visualize(*ETG::RenderContext::Window);

    if (Impacting)
    {
        // Impact frame'leri farklı boyutlardadır -- knav3 seti 6x6'dan 16x17'ye büyüyüp 9x8'e iner -- ve sheet'e
        // üst kenarlarından hizalanarak dikilir. Origin bu yüzden sabit bir değer değil, o frame'in kendi
        // merkezidir; sabit olsaydı patlama büyüdükçe yukarı sola kayardı.
        //
        // Rotation verilmez: impact artwork'ü dikey author edilmiştir ve merminin geliş açısıyla döndürülmesi
        // istenmiyor.
        const ETG::IntRect& frame = ImpactAnim.CurrRect;
        ImpactAnim.Draw(ImpactPos, ETG::Color::White, 0.f,
                        {static_cast<float>(frame.width) / 2.f, static_cast<float>(frame.height) / 2.f},
                        {1.f, 1.f}, 0.f);
        return;
    }

    auto& DrawableProps = GetDrawProperties();
    ETG::Sprite frame;
    frame.setTexture(*Texture);
    frame.setOrigin(frame.getTexture()->getSize().x / 2, frame.getTexture()->getSize().y / 2);
    frame.setPosition(DrawableProps.Position);
    frame.setRotation(DrawableProps.Rotation);
    ETG::GlobSpriteBatch.Draw(frame, 0);
}
