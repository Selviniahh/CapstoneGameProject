#include "RogueSpecial.h"
#include <filesystem>
#include "../../../Engine/Core/Factory.h"
#include "../../../Engine/Managers/AssetManager.h"
#include "../Base/HandRig.h"
#include "../VFX/ShellEjector.h"



ETG::RogueSpecial::RogueSpecial(const ETG::Vector2f& Position) : GunBase(Position,
0.35f,
200.f,
300.f,
0.f,
3.f,
300,
10,
2.0f,
3.5f,
35.f,
0.2f,
10.f)
{
    AnimationComp = CreateGameObjectAttached<RogueSpecialAnimComp>(this);
    SetShootSound(AssetManager::Resolve("Sounds/RogueSpecialShoot.ogg"));
    SetReloadSound(AssetManager::Resolve("Sounds/Reload.ogg"));

    // Ortak initialization işlemini çağır.
    RogueSpecial::Initialize();
}

void ETG::RogueSpecial::Initialize()
{
    GunBase::Initialize();

    ArrowComp->arrowOffset = {20.f, -6.f};

    // 12x10 idle frame'inin Origin'i RogueSpecialAnimComp::AttachmentOrigin ile {1,10}'a, yani grip'in alt ucuna
    // sabitlenir. Bu nedenle gun-local {0,0} zaten elin kavradığı pixel'dir; anchor'ın onu kaydırması gerekmez.
    // Revolver tek elle tutulduğundan yalnızca trigger hand silaha attach edilir; off hand body üzerinde kalır.
    //
    // NOTE: Anchor yalnızca eli silahın üzerine koymak için değil, aşağıdaki shot kick'i için de gereklidir. Anchor
    // olmadan el HoldPoint üzerinde durur ve silahın kick'ini takip etmez; el ile silah birbirinden ayrılırdı.
    Hands->RightHandAnchor = {0.f, 0.f};
    Hands->HasRightHandAnchor = true;
    Hands->HasLeftHandAnchor = false;

    // <---------- Shot performansı ---------->
    // Ateş anında kavrayışın tamamı küçük bir daire çizip yerine döner; aynı anda boştaki el nefes hattının üstüne
    // bir kez çıkıp iner. İkisi de tek bir saatle sürülür, çünkü ikisi de aynı olayın parçasıdır.
    // FireRate 0.35 olduğundan tur, sonraki shot onu kesmeden rahatça biter.
    Hands->ShotKick.Duration = 0.1f;
    Hands->ShotKick.GunCircleRadius = 1.25f; // kavrayış toplamda iki radius kadar geriye gider
    Hands->ShotKick.OffHandRise = 2.f;

    // <---------- Boştaki elin duruşu ---------->
    // Silahı tutmayan el body üzerinde durur ama ölü değildir: ateş edilsin edilmesin sürekli nefes alıp verir.
    Hands->OffHandBreath.Height = 1.5f;    // rest pose'un kaç pixel altına indiği
    Hands->OffHandBreath.Period = 0.7f;     // bir tam iniş + çıkış süresi
    Hands->OffHandBreath.FallRatio = 0.5f;  // büyütmek yavaşça indirip hızla kaldırır

    // Boştaki eli gövdeye doğru içeri alır. Rest pose'u gövdenin karşı tarafında, merkezden 7-8 pixel uzaktadır ve
    // bu değer onu içeri çeker. Sağ tarafa göre author edilir; silah sola geçtiğinde X mirror edildiğinden
    // "gövdeye doğru" iki tarafta da gövdeye doğru kalır.
    Hands->EmptyHandOffset = {2.5f, 0.f};

    // Revolver tek elle tutulur, yani boştaki el gövdenin önünde serbest durur. Hero arkasını döndüğünde onu
    // saklamak gerekir; varsayılan set zaten üç back facing'dir, o yüzden burada ayarlanacak bir şey yok.
    Hands->HideOffHandIn.UpLeft = true;
    Hands->HideOffHandIn.UpRight = true;

    // <---------- Kovanlar ---------->
    // 12x10 idle frame'inde cylinder'ın üstü, AnchorOrigin {1,10} çıkarılmış hâliyle. Gerçek bir revolver ateş
    // ederken kovan atmaz ama istenen her shot'ta kovan düşmesi; Enabled'ı false yapmak onu kapatır.
    Shells->EjectPoint = {4.f, -7.f}; // Idle pixel {5,3} - AnchorOrigin {1,10}
    Shells->EjectVelocity = {-12.f, -38.f};

    // 3x2 revolver kovanı: tüfeğinkinden bir pixel kısa ve dibinde bakır kapsül var. İkisi aynı odada birikince
    // yerdeki kovanlardan hangi silahın ateşlendiği okunur.
    Shells->SetSprite("Guns/RogueSpecial/RogueSpecialShell.png");
    Shells->SetScale({0.5f, 0.5f});
    
    // Artwork'ü sabit elin altından iki pixel aşağı kaydırır. Bu bir world-space slide'dır ve Origin'e
    // dokunmadığından silahın nişan alırken döndüğü pivot da, herhangi bir aim angle'daki oryantasyonu da
    // değişmez. WorldPointOnGun bu kaymayı geri aldığı için el olduğu yerde kalır; hareket eden yalnızca sprite'tır.
    HeldOffset = {0.5f, 2.f};

    // El değişimini, body facing'in döndüğü 67.5 derecede değil, barrel dikey konumu geçtiği anda yapar.
    // Revolver yeterince kısa olduğundan ikisinin uyuşmadığı 22.5 derecelik aralıkta sprite'ın önceden
    // mirror edilip muzzle'ın hâlâ sağa bakması açıkça görünüyordu.
    HandSwapAngle = 90.f;

    // Muzzle flash animation'ını ayarla.
    MuzzleFlash->SetAnimation("Guns/RogueSpecial/MuzzleFlash/", "RS_muzzleflash_001", "png", 0.10f);
    MuzzleFlash->SetAttachmentOffset({37.f, -6.f});
    MuzzleFlash->Deactivate();

    // Reload VFX. MuzzleFlash varsayılan olarak origin'i birleştirilmiş sprite sheet'in merkezine koyar.
    // Bu, multi-frame strip için anlamlı olmadığından her ikisi de origin'i kendi frame'lerinde
    // attachment point üzerine gelmesi gereken pixel'e sabitler.
    ReloadFlash = CreateGameObjectAttached<class MuzzleFlash>(this);
    ReloadFlash->SetAnimation("Guns/RogueSpecial/Reload/MuzzleFlash/", "RogueSpecial_Reload_MuzzleFlash_001", "png", 0.06f);
    ReloadFlash->SetParent(this);
    ReloadFlash->SetAttachmentOffset({27.f, -8.f}); // Barrel ucu
    ReloadFlash->SetOrigin({15.f, 8.5f});                 // Frame'ler merkezlenerek çizilir
    ReloadFlash->Deactivate();

    ReloadSmoke = CreateGameObjectAttached<class MuzzleFlash>(this);
    ReloadSmoke->SetAnimation("Guns/RogueSpecial/Reload/", "RogueSpecial_reload_smoke_001", "png", 0.12f);
    ReloadSmoke->SetParent(this);
    ReloadSmoke->SetAttachmentOffset({13,-10}); // Açık cylinder'ın altı
    ReloadSmoke->SetOrigin({2.f, 11.f});                  // Dumanın root pixel'i
    ReloadSmoke->SetInheritParentRotation(false); // Silah sola nişan alırken yükselmeye devam etsin
    ReloadSmoke->Deactivate();
    

    // RogueSpecial için projectile texture'ını yükle.
    ProjTexture = AssetManager::LoadTexture("Projectiles/RogueSpecial/Projectile_RogueSpecial.png");

    // Mermi bir enemy'ye değdiğinde veya menzili bittiğinde oynayan çarpma efekti. GunBase'in verdiği ortak
    // impact_tiny'nin yerini alır; 7 frame toplamda 0.21 saniye sürer ve bittiği anda projectile yok edilir.
    //
    // NOTE: Set'in vertical olanı seçilir. Yanındaki Horizontal klasörü duvarın yan yüzü içindir ve duvar
    // collision'ı girene kadar kullanılmaz.
    SetProjectileImpact("Projectiles/RogueSpecial/Impact/Vertical/", "knav3_impact_vertical_001", "png", 0.03f);

}

void ETG::RogueSpecial::Update()
{
    GunBase::Update();

    ReloadFlash->Update();
    ReloadSmoke->Update();

    if (LastShot.ShotCount > 1)
    {
        // Burst ateşlerken flash animation'ını bullet frequency ile eşleştir
        MuzzleFlash->Animation.FrameInterval = ShotDelay / 2;
    }
    else
    {
        // Single shot için normal animation hızı
        MuzzleFlash->Animation.FrameInterval = FireRate / 3;
    }
}

void ETG::RogueSpecial::Draw()
{
    GunBase::Draw();

    // Hero dash yaparken IsVisible temizlenir; silah gizlenir ancak projectile'ları çizilmeye devam eder.
    // Reload VFX, projectile'lar yerine silahı takip etmelidir.
    if (!IsVisible) return;
    if (ReloadFlash->IsVisible) ReloadFlash->Draw();
    if (ReloadSmoke->IsVisible)
        ReloadSmoke->Draw();
}

void ETG::RogueSpecial::Reload()
{
    const bool wasReloading = IsReloading;

    GunBase::Reload();

    // Magazine zaten doluysa veya reload çalışıyorsa GunBase::Reload işlemi iptal eder;
    // bu yüzden VFX'i yalnızca reload gerçekten başladığında oynat.
    if (!wasReloading && IsReloading)
    {
        ReloadFlash->Restart();
        ReloadSmoke->Restart();
    }
}

ETG::RogueSpecialAnimComp::RogueSpecialAnimComp()
{
    IsGameObjectUISpecified = true;
    RogueSpecialAnimComp::SetAnimations();
}

void ETG::RogueSpecialAnimComp::SetAnimations()
{
    // Idle Animation ayarları
    const Animation IdleAnim = {Animation::CreateSpriteSheet("Guns/RogueSpecial", "RogueSpecial_Idle", "png", 0.15f, true)};
    AddGunAnimationForState(GunStateEnum::Idle, Playback::Loop, IdleAnim, true,
                           AttachmentOrigin);

    // Shoot Animation ayarları
    Animation ShootAnim = {Animation::CreateSpriteSheet("Guns/RogueSpecial/Fire", "knav3_fire_001", "png", 0.15f)};
    AddGunAnimationForState(GunStateEnum::Shoot, Playback::Once, ShootAnim, true,
                           AttachmentOrigin);

    // Reload Animation: 2 saniyelik reload süresine yayılan 8 frame. Silahın front sight'ı kesilmeden
    // kick yapabilmesi için frame'lerde silahın üzerinde fazladan bir row bulunur. Bu nedenle origin
    // burada {1,11}, bu row'un bulunmadığı pose'larda ise {1,10} değerindedir.
    Animation ReloadAnim = {Animation::CreateSpriteSheet("Guns/RogueSpecial", "RogueSpecial_Reload", "png", 0.25f, true)};
    AddGunAnimationForState(GunStateEnum::Reload, Playback::Once, ReloadAnim, true,
                           AttachmentOrigin);
}
