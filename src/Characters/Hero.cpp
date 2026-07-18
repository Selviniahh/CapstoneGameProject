#include "Hero.h"
#include <filesystem>
#include "../Core/Components/CollisionComponent.h"
#include "../Core/Components/BaseHealthComp.h"
#include "../Enemy/EnemyBase.h"
#include "../Managers/GameState.h"
#include "../Managers/SpriteBatch.h"
#include "../Guns/RogueSpecial/RogueSpecial.h"
#include "../Projectile/ProjectileBase.h"
#include "../UI/UIObjects/ReloadText.h"
#include "Components/HeroAnimComp.h"
#include "Components/HeroMoveComp.h"
#include "Components/InputComponent.h"
#include "Hand/Hand.h"

float ETG::Hero::MouseAngle = 0;
ETG::Direction ETG::Hero::CurrentDirection{};
bool ETG::Hero::IsShooting{};

ETG::Hero::Hero(const ETG::Vector2f Position)
{
    this->Position = Position;
    Depth = -1;
    GameState::GetInstance().SetHero(this);

    Hand = ETG::CreateGameObjectAttached<class Hand>(this);
    RogueSpecial = ETG::CreateGameObjectAttached<class RogueSpecial>(this, Hand->GetRelativePosition());
    ReloadText = ETG::CreateGameObjectAttached<class ReloadText>(this);
    AnimationComp = ETG::CreateGameObjectAttached<HeroAnimComp>(this);
    AnimationComp->Initialize();
    AnimationComp->Update(); //Set the Texture during Initialization
    MoveComp = ETG::CreateGameObjectAttached<HeroMoveComp>(this);
    MoveComp->Initialize();
    InputComp = ETG::CreateGameObjectAttached<InputComponent>(this);
    HealthComp = ETG::CreateGameObjectAttached<BaseHealthComp>(this, 2.f); //by default health will be 4
    HealthComp->InvulnerabilityEnabled = false;

    //Collision comp:
    CollisionComp = ETG::CreateGameObjectAttached<CollisionComponent>(this);
    CollisionComp->CollisionRadius = 1.f;
    CollisionComp->SetCollisionEnabled(true);

    //Set default gun to equipped guns
    EquippedGuns.push_back(RogueSpecial.get());
    CurrentGun = EquippedGuns[0]; //Hero's default gun is RogueSpecial
    ReloadText->LinkToGun(CurrentGun);

    Hero::Initialize();
}

void ETG::Hero::Initialize()
{
    GameObjectBase::Initialize();

    //NOTE: damage taken at here
    HealthComp->OnDamageTaken.AddListener([this](const float damage, const float forceMagnitude, const GameObjectBase* instigator)
    {
        //IF dead ignore the damage
        if (CurrentHeroState == HeroStateEnum::Die) return;

        this->SetState(HeroStateEnum::Hit);
        const ETG::Vector2f knockbackDir = Math::Normalize(Position - instigator->GetPosition());
        MoveComp->ApplyForce(knockbackDir, forceMagnitude, 0.2f);
    });

    //NOTE: Hero will die at here
    HealthComp->OnDeath.AddListener([this](GameObjectBase* instigator)
    {
        this->SetState(HeroStateEnum::Die);
    });

    CollisionComp->OnCollisionEnter.AddListener([this](const CollisionEventData& eventData)
    {
        //If the collision is with enemy, apply force to our hero and damage
        if (eventData.Other->IsA<EnemyBase>())
        {
            const auto enemyObj = static_cast<EnemyBase*>(eventData.Other); //all safe so ignore (sometimes) useless clang-tidy 
            HealthComp->ApplyDamage(0.5, EnemyCollideKnockBackMag, enemyObj);
        }

        //If the collision is with enemy projectile, damage our hero and destroy the enemy projectile
        //eventData.Other->Owner = Projectile's gun. I didn't write that cuz dynamic_cast.md is expensive
        if (eventData.Other->IsA<ProjectileBase>())
        {
            auto* projectile = eventData.Other->As<ProjectileBase>();

            if (projectile && projectile->Owner && projectile->Owner->Owner &&
                projectile->Owner->Owner->IsA<EnemyBase>())
            {
                const auto enemy = static_cast<EnemyBase*>(projectile->Owner->Owner);
                if (HasAnyFlag(StateFlags, HeroStateFlags::PreventDamage)) return;

                HealthComp->ApplyDamage(0.5, HitKnockBackMagnitude, enemy);
                projectile->MarkForDestroy();
            }
        }

        if (eventData.Other->IsA<ActiveItemBase>())
        {
            auto* activeItem = eventData.Other->As<ActiveItemBase>();
            CurrActiveItem = activeItem;
        }
    });

    CollisionComp->OnCollisionExit.AddListener([this](const CollisionEventData& eventData)
    {
        //No exit required for now
    });
}

ETG::Hero::~Hero() = default;

void ETG::Hero::UpdateComponents()
{
    CollisionComp->Update();
    InputComp->Update(*this);
    MoveComp->Update();
    HealthComp->Update();
}

void ETG::Hero::UpdateAnimations()
{
    if (CanFlipAnims()) AnimationComp->FlipSpritesY<GunBase>(CurrentDirection, *CurrentGun);
    if (CanFlipAnims()) AnimationComp->FlipSpritesX(CurrentDirection, *this);
    AnimationComp->Update();
}

void ETG::Hero::UpdateHand() const
{
    const ETG::Vector2f HandOffsetForHero = AnimationComp->IsFacingRight(CurrentDirection) ? ETG::Vector2f{8.f, 5.f} : ETG::Vector2f{-7.f, 5.f};
    Hand->SetPosition(Position + Hand->HandOffset + HandOffsetForHero);
    Hand->Update();

    //If dashing or hit anim playing do not draw gun and hand
    Hand->IsVisible = CanMove();
}

void ETG::Hero::UpdateGuns() const
{
    CurrentGun->SetPosition(Hand->GetPosition() + Hand->GunOffset);
    CurrentGun->Rotation = MouseAngle;

    //If dashing or hit anim playing do not draw gun and hand
    CurrentGun->IsVisible = CanMove();

    //Update  all equipped guns (for their projectiles only)
    //NOTE: if (IsAttachedObjectNeeded()) //Calling this will act like stopping the time for projectiles. If I had some time, I'd implement stop time active item
    for (const auto guns : EquippedGuns)
        guns->Update();
}

void ETG::Hero::HandleShooting() const
{
    if (IsShooting && CurrentGun->MagazineAmmo != 0 && !CurrentGun->IsReloading && CanShoot())
    {
        CurrentGun->PrepareShooting();
    }
}

void ETG::Hero::HandleActiveItem() const
{
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::Space) && CurrActiveItem && CanUseActiveItems())
    {
        CurrActiveItem->RequestUsage();
    }
}

void ETG::Hero::Update()
{
    GameObjectBase::Update();
    UpdateComponents();
    UpdateAnimations();

    //NOTE: Reload Text: Will run only if reload needed 
    ReloadText->Update();

    HandleShooting();
    HandleActiveItem();
    UpdateHand();
    UpdateGuns();
}

void ETG::Hero::Draw()
{
    if (!IsVisible) return;
    GameObjectBase::Draw();
    SpriteBatch::Draw(GetDrawProperties());
    CurrentGun->Draw();
    ReloadText->Draw();
    Hand->Draw();

    //Draw all equipped guns (for their projectiles only)
    for (const auto guns : EquippedGuns)
        guns->Draw();

    if (CollisionComp) CollisionComp->Visualize(*GameState::GetInstance().GetRenderWindow());
}

//----------------------------State Functionalities ----------------------------
void ETG::Hero::SetState(const HeroStateEnum& state)
{
    CurrentHeroState = state;

    switch (state)
    {
    case HeroStateEnum::Idle: StateFlags = HeroStateFlags::StateIdle;
        break;
    case HeroStateEnum::Run: StateFlags = HeroStateFlags::StateRun;
        break;
    case HeroStateEnum::Dash: StateFlags = HeroStateFlags::StateDash;
        break;
    case HeroStateEnum::Die: StateFlags = HeroStateFlags::StateDie;
        break;
    case HeroStateEnum::Hit: StateFlags = HeroStateFlags::StateHit;
        break;
    }
}

//----------------------------Gun Switch Functionalities ----------------------------
ETG::GunBase* ETG::Hero::GetCurrentHoldingGun() const
{
    return CurrentGun;
}

void ETG::Hero::EquipGun(GunBase* newGun)
{
    EquippedGuns.push_back(newGun);
    CurrentGun = newGun; // Set the new gun as the current one by default
    currentGunIndex = EquippedGuns.size() - 1;
    ReloadText->LinkToGun(CurrentGun);
    UpdateGunVisibility();
}

void ETG::Hero::SwitchGun(const int& index)
{
    // First check if we have any guns at all
    if (EquippedGuns.empty()) return;

    // Move index backwards -1 or +1 with wraparound
    // No need for additional bounds check - the modulo operation guarantees the index is valid if the vector is not empty
    currentGunIndex = (currentGunIndex + index + EquippedGuns.size()) % EquippedGuns.size();
    CurrentGun = EquippedGuns[currentGunIndex];
    ReloadText->LinkToGun(CurrentGun);
    ReloadText->SetNeedsReload(CurrentGun->IsMagazineEmpty());
    UpdateGunVisibility();
}

void ETG::Hero::SwitchToPreviousGun()
{
    SwitchGun(-1);
}

//Gun switching
void ETG::Hero::SwitchToNextGun()
{
    SwitchGun(1);
}

void ETG::Hero::UpdateGunVisibility() const
{
    for (GunBase* gun : EquippedGuns)
    {
        gun->IsVisible = (gun == CurrentGun);
    }
}
