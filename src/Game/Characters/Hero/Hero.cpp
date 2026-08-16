#include "Hero.h"
#include <filesystem>
#include <imgui.h>
#include "../../../Engine/Editor/UI/UIUtils.h"
#include "../../../Engine/Managers/Time.h"
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "../../../Engine/Core/Components/BaseHealthComp.h"
#include "../Enemy/EnemyBase.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../Items/Active/ActiveItemBase.h"
#include "../../Items/Passive/PassiveItemBase.h"
#include "../../../Engine/Managers/SpriteBatch.h"
#include "../../Guns/RogueSpecial/RogueSpecial.h"
#include "../../Projectile/ProjectileBase.h"
#include "../../UI/UIObjects/ReloadText.h"
#include "Components/HeroAnimComp.h"
#include "Components/HeroMoveComp.h"
#include "Components/InputComponent.h"
#include "Hand/Hand.h"

ETG::Hero::Hero(const ETG::Vector2f Position)
{
    this->Position = Position;
    Depth = -1;

    //The hero's art puts the left hand one pixel further in than the shared default
    HandOffsetLeft = {-7.f, 5.f};

    //NOTE: Built before the components because they ask the hero for its state while constructing (HeroAnimComp
    //does, in its constructor). Building and entering the tree touches no component, so this ordering is safe
    StateMachine = std::make_unique<HeroStateMachine>();
    StateMachine->Build();
    StateMachine->Start(*this);

    Hand = ETG::CreateGameObjectAttached<class Hand>(this);
    OffHand = ETG::CreateGameObjectAttached<class Hand>(this);
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
    CollisionComp->Name = "Hero";

    //Frame'ler 23x24, ama icine cizilmis rogue yaklasik 15x21 - her iki yanda dorder sutun bos piksel var.
    //Yani sanattan alinan bounds her iki yonde dort piksel fazla genis, ve duvar soz konusu oldugunda bu
    //gormezden gelinebilecek bir yuvarlama hatasi degil: hero, tasla arasinda GOZLE GORULUR bir bosluk birakarak
    //duruyor. Elle yazilan kutu, cizildigi frame yerine govdenin kendisini temsil ediyor; UseManualBounds zaten
    //tam olarak bu durum icin var. Iki sayi da editorde canli, yani his surukleyerek ayarlaniyor, rebuild ile degil.
    //
    //NOTE: bu kutu SADECE duvarlar icin degil, HER SEY icin gecerli - dusmanin sana degmek icin ulasmasi gereken
    //ve merminin isabet etmek icin ulasmasi gereken kutu da bu. Onlar da bununla birazcik daraldi; ki bu ayni
    //duzeltmenin parcasi: oncesinde dort piksellik hicligi vuruyorlardi
    CollisionComp->UseManualBounds = true;
    CollisionComp->ManualBoundsSize = {12.f, 18.f}; //CollisionRadius genislettikten sonra 14x20 - 16'lik tile'in altinda, yani tek hucrelik bosluktan gecilebiliyor

    //Vucut temasi icin dusmanlar, vurulmak icin mermiler. Silahlar ve itemlar bilerek yok: uzerlerinden gecince
    //alinmalari o objenin kendi listener'inin isi, dolayisiyla burada calisacak bir sey olmazdi
    CollisionComp->Layer = CollisionLayer::Hero;
    CollisionComp->Mask = CollisionLayer::Enemy | CollisionLayer::Projectile;

    //Obstacle yukaridaki Mask'ta DEGIL, ve ikisinden hicbirine de ait degil: duvar, icine yurudukten sonra haber
    //verilecek bir sey degil. Onun yerine burada isimlendiriliyor - hareket pasinin, yuruyus gerceklesmeden ONCE
    //okudugu yerde
    CollisionComp->BlockingMask = CollisionLayer::Obstacle;

    CollisionComp->SetCollisionEnabled(true);

    //Bounds ilk collision pasinin sonunda degil, simdi hazir olsun - hareket sorgusu onlari frame'in ortasindan
    //okuyor, ve ilk frame'de bu, pas hic calismadan onceki an demek
    CollisionComp->Initialize();

    //Ve isin oteki yarisi: mover'a, dunyanin durdurmasina izin verilen kutunun hangisi oldugu soylenmeli. Burada
    //set ediliyor, HeroMoveComp'un constructor'inda degil; cunku o noktada collider henuz yok
    MoveComp->BodyCollider = CollisionComp.get();

    //Hero's default gun is RogueSpecial. EquipGun files it in the inventory and points the reload UI at it
    EquipGun(RogueSpecial.get());

    Hero::Initialize();

    //Last statement, and the constructor's alone: see GameObjectBase::BindEvents
    Hero::BindEvents();
}

void ETG::Hero::Initialize()
{
    GameObjectBase::Initialize();
}

void ETG::Hero::BindEvents()
{
    //NOTE: Hero overlapped by something
    CollisionComp->OnCollisionEnter.AddListener([this](const CollisionEventData& eventData)
    {
        //NOTE: If collided with the enemy
        if (eventData.Other->IsA<EnemyBase>())
        {
            //No projectile to hand over: this is body contact, so a modifier can only swallow it
            if (ConsumeIncomingDamage(nullptr)) return;

            const auto enemyObj = eventData.Other->As<EnemyBase>();
            HealthComp->ApplyDamage(0.5, EnemyCollideKnockBackMag, enemyObj);
        }

        //NOTE: If collided with a projectile
        if (eventData.Other->IsA<ProjectileBase>())
        {
            return;
            auto* projectile = eventData.Other->As<ProjectileBase>();

            if (projectile && projectile->Owner && projectile->Owner->Owner &&
                projectile->Owner->Owner->IsA<EnemyBase>())
            {
                //NOTE: whatever the modifiers want to do with the shot - eat it, bounce it back - they do here and
                //this stays one line. Hero deliberately knows no concrete modifier: adding the next effect means
                //writing that modifier, not editing this listener again
                if (ConsumeIncomingDamage(projectile)) return;

                const auto enemy = static_cast<EnemyBase*>(projectile->Owner->Owner);
                if (!CanTakeDamage()) return;

                HealthComp->ApplyDamage(0.5, HitKnockBackMagnitude, enemy);
                projectile->MarkForDestroy();
            }
        }

        //NOTE: picking an item up is the item's own listener now (Character::PickUpActiveItem), so that an enemy
        //walking over the same item picks it up by the identical path
    });

    //NOTE: damage taken at here by HealthComp
    HealthComp->OnDamageTaken.AddListener([this](const float damage, const float forceMagnitude, const GameObjectBase* instigator)
    {
        RequestHit(Math::Normalize(Position - instigator->GetPosition()), forceMagnitude);
    });

    CollisionComp->OnCollisionExit.AddListener([this](const CollisionEventData& eventData)
    {
        //No exit required for now
    });
}

ETG::Hero::~Hero() = default;

ETG::HeroMoveComp* ETG::Hero::GetMoveComp() const
{
    return GetMoveCompAs<HeroMoveComp>();
}

void ETG::Hero::UpdateComponents()
{
    InputComp->Update(*this);
    MoveComp->Update();
    HealthComp->Update();
}

void ETG::Hero::UpdateAnimations()
{
    //Only the body is flipped here. The gun's own mirror follows the hand it is in, which is Character::UpdateGuns'
    //call to make - a gun may change hands at an angle the body's 8-way facing knows nothing about
    if (CanFlipAnims()) AnimationComp->FlipSpritesX(CurrentDir, *this);
    AnimationComp->Update();
}

void ETG::Hero::HandleShooting() const
{
    if (IsShooting && CurrentGun && CurrentGun->MagazineAmmo != 0 && !CurrentGun->IsReloading && CanShoot())
    {
        CurrentGun->PrepareShooting();
    }
}

//NOTE: the key binding is the only hero-specific part; whether the item may fire at all is Character's call
void ETG::Hero::HandleActiveItemInput() const
{
    if (ETG::Keyboard::isKeyPressed(ETG::Keyboard::Space))
        UseActiveItem();
}

void ETG::Hero::Update()
{
    GameObjectBase::Update();

    //Order matters: the components gather input and resolve forces, the machine then decides the state and runs the
    //behaviour belonging to it, and only then do the animations render whatever state we ended up in
    UpdateComponents();
    StateMachine->Tick(*this, Time::FrameTick);
    ExpireRequests();
    UpdateAnimations();

    //NOTE: Reload Text: Will run only if reload needed
    ReloadText->Update();

    HandleShooting();
    HandleActiveItemInput();
    UpdateHoldPoint();
    UpdateGuns();
    UpdateHandAndGunVisibility();
}

void ETG::Hero::Draw()
{
    if (!IsVisible) return;
    GameObjectBase::Draw();
    SpriteBatch::Draw(GetDrawProperties());
    ReloadText->Draw();

    //Draw all equipped guns (the holstered ones only draw their projectiles)
    for (const auto guns : EquippedGuns)
        guns->Draw();

    //Hands are always submitted by the character after its guns, so an equipped gun cannot cover them at the
    //same depth. Whether the off hand participates is still the concrete gun's decision through HasLeftHandAnchor.
    Hand->Draw();
    OffHand->Draw();

    if (CollisionComp) CollisionComp->Visualize(*ETG::RenderContext::Window);
}

//The reload UI belongs to the player, so it follows the gun from here rather than from the inventory code
void ETG::Hero::OnGunChanged(GunBase* gun)
{
    if (!ReloadText || !gun) return;

    ReloadText->LinkToGun(gun);
    ReloadText->SetNeedsReload(gun->IsMagazineEmpty());
}

//----------------------------State Functionalities ----------------------------
void ETG::Hero::RequestDash(const HeroDashEnum direction)
{
    if (direction == HeroDashEnum::Unknown) return;

    //NOTE: CurrentDashDirection is not only the direction being asked for, it is also what HeroAnimComp resolves
    //the dash animation from. Overwriting it mid-dash therefore swaps the animation under a dash that has already
    //committed to a direction, so a request filed while dashing is refused here rather than only at the transition
    if (GetState() == HeroStateEnum::Dash) return;

    CurrentDashDirection = direction;
    DashRequested = true;
}

//A request is worth exactly one tick. Input re-files RequestDash on every frame the right mouse button is held, so
//a request the machine found no legal transition for - the hero is already dashing, or the cooldown has not run out
//yet - must not survive to fire itself later, long after the player let go. Holding the button still chains dashes,
//because input keeps filing a fresh request; it is only the stale ones that are dropped
void ETG::Hero::ExpireRequests()
{
    DashRequested = false;
    HitRequested = false;
}

void ETG::Hero::RequestHit(const ETG::Vector2f& knockbackDir, const float forceMagnitude)
{
    //NOTE: This is the old `if (CurrentHeroState == Die) return;` from the damage listener. Dead is a terminal
    //subtree, so a request filed from in there would sit around unconsumed forever
    if (!StateMachine->IsAlive()) return;

    PendingKnockbackDir = knockbackDir;
    PendingKnockbackForce = forceMagnitude;
    HitRequested = true;
}

void ETG::Hero::PopulateSpecificWidgets()
{
    GameObjectBase::PopulateSpecificWidgets();

    //NOTE: The old UI showed a single CurrentHeroState enum. The full path is strictly more informative:
    //it tells you which subtree you are in, which is what decides the hero's capabilities
    UIUtils::BeginProperty("State Path");
    ImGui::Text("%s", StateMachine->GetActivePathName().c_str());
    UIUtils::EndProperty();

    UIUtils::BeginProperty("Time In State");
    ImGui::Text("%.3f s", StateMachine->TimeInState());
    UIUtils::EndProperty();
}

//Offers the hit to every attached modifier and stops at the first one that claims it
bool ETG::Hero::ConsumeIncomingDamage(ProjectileBase* projectile)
{
    //Ask each attached modifier in turn; the first one that claims the hit ends it
    for (const auto& [type, modifier] : HeroModifierManager)
        if (modifier->ReflectProjectile(*this, projectile))
            return true;

    return false;
}
