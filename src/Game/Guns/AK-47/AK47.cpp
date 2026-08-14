#include "AK47.h"
#include <algorithm>
#include <filesystem>
#include "../../../Engine/Core/Factory.h"
#include "../../../Engine/Core/Components/CollisionComponent.h"
#include "../../Characters/Hero/Hero.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Managers/AssetManager.h"
#include "../VFX/MagazineDrop.h"
#include "../Base/HandRig.h"
#include "../VFX/ShellEjector.h"
#include "../../../Utils/Math.h"

ETG::AK47::AK47(const ETG::Vector2f& pos) : GunBase(pos,
    0.3f,     // FireRate
    200.0f,     // ShotSpeed
    1000.0f,    // Range (infinite olmalı ama şimdilik 2000 vereceğim)
    0.0f,      // timerForVelocity
    -2.f,      // depth
    500,       // MaxAmmo
    30,        // MagazineSize
    2.0f,      // ReloadTime
    1.0f,      // Damage
    20.0f,     // Force
    0.1f,     // Force duration
    3.0f)      // Spread (degree cinsinden)
{
    AnimationComp = CreateGameObjectAttached<AK47AnimComp>(this);
    SetShootSound(AssetManager::Resolve("Sounds/AK47Shoot.ogg"));
    SetReloadSound(AssetManager::Resolve("Sounds/AK47Reload.ogg"));
    
    CollisionComp = ETG::CreateGameObjectAttached<CollisionComponent>(this);

    //A gun on the floor is only ever waiting for the hero to walk into it
    CollisionComp->Layer = CollisionLayer::Pickup;
    CollisionComp->Mask = CollisionLayer::Hero;

    // Ortak initialization işlemini çağır
    AK47::Initialize();
}

void ETG::AK47::Initialize()
{
    ArrowComp->arrowOriginOffset = {-6.f, 0.f};
    ArrowComp->arrowOffset = {15.f, -2.f};

    // 27x7 idle frame'de sol üstten okunan {17,3} ve {7,5} noktaları, frame'in merkez Origin'i
    // {13.5,3.5} çıkarılarak gun-local uzayda saklanır. Trigger hand grip üzerinde, diğer el ise
    // magazine'in önündedir. Rifle iki elle tutulduğu için iki anchor da aktiftir.
    // TODO: Aynı silahı enemy'nin de kullanmasını isteyeceğim. Bu yüzden owner hero ise bunu yap;
    // owner BulletMan veya başka bir enemy ise ilgili davranışla devam et.
    Hands->RightHandAnchor = {3.5f, -0.5f};
    Hands->LeftHandAnchor = {-6.5f, 1.5f};
    Hands->HasRightHandAnchor = true;
    Hands->HasLeftHandAnchor = true;

    // <---------- Reload performansı ---------->
    // Öndeki el handguard'dan ayrılır, grip yanındaki magazine well'e uzanır, bir kez yukarı çekip geri yerleşir;
    // trigger hand grip'ten ayrılmaz, yalnızca eğilen reload pose'unun grip'ini takip eder. Hareketin kendisi
    // HandRig::ReloadReach içindedir; burada yalnızca AK'ye özgü noktalar ve zamanlama author edilir.
    //
    // İki nokta da 26x10 reload frame'inin sol üstünden okunur ve HandRig::AnchorOrigin, yani 27x7 idle frame'inin
    // merkezi {13.5,3.5} çıkarılır. Reload frame'inin kendi Origin'i DEĞİL: anchor'larla aynı referansa göre
    // yazıldıklarında `WorkingPoint - RightHandAnchor` gibi bir çıkarma anlamlı kalır ve iki uzay karışmaz.
    // Reload frame'i çizilirken oluşan Origin farkını WorldPointOnGun zaten geri ekler.
    Hands->ReloadReach.Enabled = true;
    Hands->ReloadReach.WorkingPoint = {-9.5f, 3.5f}; // Reload pixel {4,7} - AnchorOrigin {13.5,3.5}: magazine well
    Hands->ReloadReach.SteadyPoint = {-6.5f, 3.5f};  // Reload pixel {7,7} - AnchorOrigin: eğilen pose'daki grip

    // Body'nin 8-way facing sisteminin döndüğü 67.5 derecede dönmek yerine barrel doğrudan aşağı bakana kadar
    // sağ elde kalır. Rifle bu aralık için en kötü durumdur: hâlâ sağa nişan alırken mirror edilmesi, uzunluğu
    // nedeniyle barrel'ın tamamını hero'nun üzerinden geçirir.
    HandSwapAngle = 90.f;

    // Hero arkasını döndüğünde onun arkasında (-1), diğer durumlarda önünde kalır. Ayrıca ellerin back depth'inin
    // gerisinde durur; böylece rifle, onu kavrayan parmakları kapatmaz.
    HeldDepthBehindBody = 1.f;

    // Rifle iki elle tutulduğundan barrel yatay çizginin üzerine çıktığında forward grip body'ye sabitlenir.
    // Geri kalanını GunBase'in default rule'ları karşılar; override edilecek bir şey yoktur.
    Hands->PinsGripWhenAimingUp = true;

    // <---------- Kovanlar ---------->
    // 27x7 idle frame'inde receiver'ın üst kenarındaki ejection port'u, AnchorOrigin {13.5,3.5} çıkarılmış hâliyle.
    // Gözle oturtmak için ImGui'da EjectPoint'in yanındaki Visualize kutucuğunu tikle.
    Shells->EjectPoint = {-1.5f, -3.f}; // Idle pixel {12,2} - AnchorOrigin
    Shells->EjectVelocity = {-45.f, -44.f};

    // 4x2 tüfek kovanı. Magazine'de olduğu gibi object GunBase'e aittir; buradan yalnızca hangi kovanın
    // düşeceği söylenir.
    Shells->SetSprite("Guns/AK47/AK47Shell.png");
    Shells->SetScale({0.5f, 0.5f});


    CollisionComp->Initialize();

    // Object'in owner'ı GunBase'dir; bu silahın yalnızca hangi magazine'in düşeceğini belirtmesi gerekir
    Magazine->SetSprite("Guns/AK47/ak47_clip_001.png");

    // AK-47 için projectile texture'ını yükle
    ProjTexture = AssetManager::LoadTexture("Projectiles/AK-47/Projectile_AK-47.png");

    CollisionComp->OnCollisionEnter.AddListener([this](const CollisionEventData& eventData)
    {
        if (auto* hero = dynamic_cast<Hero*>(eventData.Other))
        {
            hero->EquipGun(this);
            CollisionComp->SetCollisionEnabled(false); // Equip sonrasında
            this->Owner = hero; // Silahın owner'ını hero yap. Projectile collision sırasında projectile owner'ını bilmemiz gerektiği için bu önemlidir.
        }
    });

    GunBase::Initialize();
}

void ETG::AK47::Update()
{
    
    MuzzleFlash->Deactivate();
    MuzzleFlash->IsVisible = false;
    ArrowComp->Update();
    GunBase::Update();

    // Ellerin reload performansını GunBase::Update içinde HandRig çalıştırdı; geriye yalnızca performansın hangi
    // anında magazine'in ayrılacağı kalır. El well'e ulaştığı anda ayrılır: tüm gesture bu beat üzerinden okunur,
    // el çeker ve çektiği nesne düşerek uzaklaşır.
    if (IsReloading && !MagazineEjected && ReloadProgress() >= Hands->ReloadReach.GrabEnd) EjectMagazine();
}

void ETG::AK47::Draw()
{
    GunBase::Draw();
}

void ETG::AK47::Reload()
{
    GunBase::Reload();
}

void ETG::AK47::EjectMagazine()
{
    MagazineEjected = true;

    // Silahla birlikte mirror edilir; böylece sola nişan alırken magazine hero'nun içinden değil,
    // her zaman rifle'ın arkasından dışarı fırlatılır.
    ETG::Vector2f velocity = MagazineEjectVelocity;
    if (!IsHeldOnRightHand) velocity.x = -velocity.x;

    // Silahın kendi depth değerinde bırakılır. Tam olarak well'in bulunduğu yerde başladığından, göründüğü
    // frame'de rifle'ın önüne veya arkasına sıçramak yerine onunla birlikte sort edilmelidir.
    Magazine->Drop(WorldPointOnGun(MagazineEjectPoint), Rotation, velocity, Depth);
}

ETG::AK47AnimComp::AK47AnimComp()
{
    IsGameObjectUISpecified = true;
    AK47AnimComp::SetAnimations();
}

void ETG::AK47AnimComp::SetAnimations()
{
    BaseAnimComp<GunStateEnum>::SetAnimations();
    
    // Interval = frame başına süre; animation'ın toplam süresi değildir
    const Animation IdleAnim = {Animation::CreateSpriteSheet("Guns/AK47", "AK47_Single001", "png", 0.15f, false)};
    AddGunAnimationForState(GunStateEnum::Idle, Playback::Loop, IdleAnim);

    // Shoot Animation ayarları
    Animation ShootAnim = {Animation::CreateSpriteSheet("Guns/AK47", "ak47_shoot_001", "png", ShootAnimInterval)};
    AddGunAnimationForState(GunStateEnum::Shoot, Playback::Once, ShootAnim);

    // Reload Animation. Bilerek tek frame kullanılır: magazine-out pose reload boyunca korunur.
    // NOTE: İki frame'li ak47_reload_001 sprite sheet, düşen magazine'i ikinci frame'e gömer. Bu magazine,
    // gerçekten düşen MagazineDrop yanında silahın altında sabit kalırdı; biri donmuş iki magazine görünürdü.
    // ak47_reload_pose_001 bu sprite sheet'in yalnızca ilk frame'idir. Böylece artwork silahı magazine olmadan
    // gösterir ve düşen nesne AK47::EjectMagazine tarafından fırlatılan object olur.
    Animation ReloadAnim = {Animation::CreateSpriteSheet("Guns/AK47", "ak47_reload_pose_001", "png", ReloadAnimInterval, false)};
    AddGunAnimationForState(GunStateEnum::Reload, Playback::Once, ReloadAnim);
    
    // Recoil Animation. Shoot Animation sonrasında bir kez oynar, ardından GunBase yeniden Idle'a geçer.
    Animation RecoilAnim = {Animation::CreateSpriteSheet("Guns/AK47", "ak47_shoot_recoil_001", "png", RecoilAnimInterval, false)};
    AddGunAnimationForState(GunStateEnum::Recoil, Playback::Once, RecoilAnim);
}
//
// │        │ kare │ interval │  teslim süresi   │
// ├────────┼──────┼──────────┼──────────────────┤
// │ Shoot  │ 3    │ 0.08     │ 2 × 0.08 = 0.16s │
// ├────────┼──────┼──────────┼──────────────────┤
// │ Recoil │ 3    │ 0.10     │ 2 × 0.10 = 0.20s │
// ├────────┼──────┼──────────┼──────────────────┤
// │        │      │          │ toplam 0.36s     │
// └────────┴──────┴──────────┴──────────────────┘

// Zaten 0.4 saniyelik süremiz vardı
