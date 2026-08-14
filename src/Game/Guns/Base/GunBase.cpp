#include "../../../Engine/Managers/Time.h"
#include <filesystem>
#include "GunBase.h"
#include <cmath>
#include <random>

#include "../../Characters/Hero/Hero.h"
#include "../../Projectile/ProjectileBase.h"
#include "../../../Engine/Managers/RenderContext.h"
#include "../../../Engine/Managers/SpriteBatch.h"
#include "../../../Engine/Core/Factory.h"
#include "../../UI/UIObjects/ReloadSlider.h"
#include "../../../Utils/Math.h"
#include "../../Items/Passive/PlatinumBullets.h"
#include "../../../Engine/Managers/AssetManager.h"
#include "../../Guns/VFX/MagazineDrop.h"
#include "HandRig.h"
#include "../VFX/ShellEjector.h"

namespace ETG
{
    GunBase::GunBase(const ETG::Vector2f Position,
                     const float fireRate,
                     const float shotSpeed,
                     const float range,
                     const float timerForVelocity,
                     const float depth,
                     const int maxAmmo,
                     const int magazineSize,
                     const float reloadTime,
                     const float damage,
                     const float force,
                     const float forceDuration,
                     const float spread)
        // NOTE: Stat'a atanan number onun base değerini belirler; dolayısıyla bunlar silahın modifier uygulanmamış
        // değerleridir. Önceden body içinde her BaseX'i X'e kopyalayan block, ikiz field'larla birlikte kaldırıldı.
        : FireRate(fireRate), ShotSpeed(shotSpeed), Range(range), ReloadTime(reloadTime),
          Damage(damage), Force(force), ForceDuration(forceDuration), Spread(spread),
          MagazineSize(magazineSize), MaxAmmo(maxAmmo), Timer(timerForVelocity)
    {
        // Ortak position ve texture'ları initialize et
        this->Position = Position;
        this->Depth = depth;

        // Her ikisi de authored depth ile başlatılır. Böylece holder'ın her frame yaptığı swap, silah gerçekten
        // holder'ın arkasında gizlenmeyi isteyene kadar no-op olur.
        HeldDepthInFront = depth;
        HeldDepthBehindBody = depth;

        MagazineAmmo = MagazineSize.GetInt(); // Magazine, MagazineAmmo ile başlamalıdır

        if (!Texture) Texture = std::make_shared<ETG::Texture>();
        if (!ProjTexture) ProjTexture = std::make_shared<ETG::Texture>();
        if (!Texture) Texture = std::make_shared<ETG::Texture>();
        if (!ArrowComp) ArrowComp = CreateGameObjectAttached<class ArrowComp>(this, AssetManager::Resolve("Projectiles/Arrow.png"));
        // Bilerek boş oluşturulur. Hangi sprite sheet'in oynatılacağı ilgili silahın sorumluluğundadır;
        // bunu kendi AnimComp::SetAnimations içinde belirtir ve GunBase::Initialize uygular.
        if (!MuzzleFlash) MuzzleFlash = CreateGameObjectAttached<class MuzzleFlash>(this);
        ReloadSlider = ETG::CreateGameObjectAttached<class ReloadSlider>(this);

        // GunBase::Update ve ::Draw bunu HER silah için tick ettiğinden, silahın owner olduğu diğer sub-object'lerle
        // birlikte burada oluşturulur. Magazine->SetSprite çağırmayan silah hiçbir şey düşürmez; MagazineDrop::Drop
        // sprite yoksa erken döner. Böylece yukarıdaki MuzzleFlash'te olduğu gibi object base'e ait kalırken artwork
        // ilgili silahın sorumluluğunda olur.
        if (!Magazine) Magazine = CreateGameObjectAttached<MagazineDrop>(this);

        // Her silahın elleri vardır, dolayısıyla rig'i de vardır. Boş bir rig hiçbir şey yapmaz: anchor'sız eller
        // body üzerinde kalır ve tüm hareketlerin genliği sıfırdır. Silah yalnızca istediğini author eder.
        if (!Hands) Hands = CreateGameObjectAttached<HandRig>(this);

        // Aynı gerekçeyle: kovan atmak istemeyen silah Enabled'ı kapatır, ötesinde hiçbir şey yapmaz
        if (!Shells) Shells = CreateGameObjectAttached<ShellEjector>(this);

        GunBase::Initialize();

        //Last statement, and the constructor's alone: see GameObjectBase::BindEvents. It matters here more than
        //anywhere - GunBase::Initialize runs twice for every gun, once from this line and once from the derived
        //gun's own Initialize, so the reload slider used to be listening to the same weapon twice
        GunBase::BindEvents();
    }

    void GunBase::BindEvents()
    {
        ReloadSlider->LinkToGun(this);
    }

    bool GunBase::IsHeldOnRightSide(const float aimAngle) const
    {
        // [-180,180] aralığına katlanır; böylece test doğrudan sağ yönüne göre symmetric olur ve 360 wrap için
        // ikinci case gerekmez. 350 derece axis'ten 350 derece uzakta değil, 10 derece yukarıdadır.
        const float signedAngle = aimAngle > 180.f ? aimAngle - 360.f : aimAngle;

        return std::abs(signedAngle) <= HandSwapAngle;
    }

    ETG::Vector2f GunBase::MirroredHeldOffset() const
    {
        // Yalnızca horizontal component mirror edilir. Böylece negatif X iki tarafta da "barrel boyunca daha geride"
        // anlamını korur. Sprite barrel ekseninde mirror edildiği için Y her iki durumda da artwork içinde yukarıdır.
        ETG::Vector2f heldOffset = HeldOffset;
        if (!IsHeldOnRightHand) heldOffset.x = -heldOffset.x;

        return heldOffset;
    }

    bool GunBase::WantsGripPinned() const
    {
        return Hands->PinsGripWhenAimingUp && IsBarrelAboveHorizontal();
    }

    void GunBase::SetProjectileImpact(const std::string& relativePath, const std::string& fileName,
                                      const std::string& extension, const float frameInterval)
    {
        ProjectileImpact = Animation::CreateSpriteSheet(relativePath, fileName, extension, frameInterval);

        // Impact bir kez oynar ve son frame'inde durur. ProjectileBase de kopyasına aynısını uygular; burada da
        // yapılması, silahın animation'ını ImGui'da inceleyenin loop eden bir şey görmemesi içindir.
        ProjectileImpact.Loops = false;

        HasProjectileImpact = true;
    }

    ETG::Vector2f GunBase::WorldPointOnGun(const ETG::Vector2f& localPoint) const
    {
        // HeldOffset silah artwork'ünü sabit ellerin altında kaydırır. Bu world-space displacement burada kaldırılır;
        // aksi hâlde silaha anchor edilmiş her şey kaymayla birlikte sürüklenir. Silah sola nişan almak için
        // mirror edildiğinde local noktayı doğru tarafta tutan şey, silahın kendi scale değerinin rotation'a verilmesidir.
        return GetPosition() - MirroredHeldOffset() +
               Math::RotateVector(GetRotation(), GetScale(), Hands->FrameAdjusted(localPoint, GetOrigin()));
    }

    float GunBase::ReloadProgress() const
    {
        return Math::Progress01(ReloadElapsed, ReloadTime.Get());
    }

    void GunBase::RemoveAllModifiersFrom(const std::string& source)
    {
 //        Loop'un her adımında stat, sıradaki object'in address'ini alır. Mantıksal olarak şuna eşdeğerdir:
 //        FireRate.RemoveModifiersFrom(source);
 //        ShotSpeed.RemoveModifiersFrom(source);
 //        Range.RemoveModifiersFrom(source);
 //        ReloadTime.RemoveModifiersFrom(source);
 //        Damage.RemoveModifiersFrom(source);
 //        Force.RemoveModifiersFrom(source);
 //        Spread.RemoveModifiersFrom(source);
 //        MagazineSize.RemoveModifiersFrom(source);
        
        for (StatModifier* stat : {&FireRate, &ShotSpeed, &Range, &ReloadTime, &Damage, &Force, &Spread, &MagazineSize})
            stat->RemoveModifiersFrom(source);
    }

    GunBase::~GunBase()
    {
        for (auto& proj : projectiles)
            proj.reset();
    }

    void GunBase::Initialize()
    {
        Timer = FireRate + 1; // Hemen ateş edebilmek için Timer'ı FireRate'ten büyük yap

        // Silah rotate edilirken handle point olan attachment point etrafında dönmesi gerektiği için Origin elle verilmelidir
        this->Origin += OriginOffset;

        // Muzzle flash sprite sheet derived gun'ın sorumluluğundadır; kendi Initialize'ı içinde SetAnimation çağırır
        MuzzleFlash->SetParent(this);

        // Ortak impact. Buraya ilk giriş GunBase constructor'ından olduğu için her silah daha kendi Initialize'ı
        // çalışmadan geçerli bir impact'e sahip olur; kendi artwork'ünü veren silahın SetProjectileImpact çağrısı
        // sonradan bunun üzerine yazar. Kontrol, Initialize ikinci kez çalıştığında seçilmiş set'in default'a geri
        // dönmemesi içindir.
        if (!HasProjectileImpact) SetProjectileImpact("Projectiles/Hit/", "impact_tiny_001", "png", 0.04f);
    }

    void GunBase::Update()
    {
        GameObjectBase::Update();

        Timer += Time::FrameTick;

        // Reload'un kendi clock'udur. Reload'un ne zaman biteceğini ReloadSlider yönetir (ReloadTime süresince
        // çalıştıktan sonra IsReloading'i temizler); bu nedenle yalnızca reload çalışırken sayması gerekir.
        if (IsReloading) ReloadElapsed += Time::FrameTick;

        // Eller. Reload clock'undan hemen sonra çalışır çünkü reload performansını süren o değerdir; Update'in geri
        // kalanından önce çalışır çünkü silahın shot kick'i position'ını kaydırır ve aşağıdaki arrow, muzzle flash
        // ve draw properties'in hepsi kaymayı aynı frame içinde görmelidir.
        //
        // NOTE: Kayma birikmez. Holder her frame Character::PlaceHeldGun içinde silahın position'ını sıfırdan yazar;
        // bu slide da her frame onun üzerine yeniden biner.
        Hands->Tick(Time::FrameTick, IsReloading ? ReloadProgress() : -1.f);
        SetPosition(GetPosition() + Math::RotateVector(Rotation, Scale, Hands->GunKickOffset()));

        // BulletQueue içindeki tüm bullet'ları ateşle ve vector'den kaldır
        if (!bulletQueue.empty())
        {
            for (auto it = bulletQueue.begin(); it != bulletQueue.end();)
            {
                it->timeToFire -= Time::FrameTick;
                if (it->timeToFire <= 0)
                {
                    // Bu bullet'ı ateşleme zamanı
                    FireBullet(it->angle);

                    it = bulletQueue.erase(it);
                }
                else
                    ++it;
            }
        }

        // Shoot bittiğinde Recoil'a, Recoil bittiğinde Idle'a geçer. Recoil Animation kaydetmemiş silah doğrudan
        // Idle'a döner. Bu nedenle başka bir silaha sonradan Recoil eklemek yalnızca kendi SetAnimations'ına bir
        // satır eklemeyi gerektirir; burada değişiklik gerekmez.
        // NOTE: Lookup find ile yapılır, operator[] ile DEĞİL. Eskiden burada `AnimManagerDict[animState]` vardı ve
        // bir okuma niyetiyle yazılmış bu satır, state kayıtlı değilse onu dict'e EKLİYORDU. Recoil animation'ı
        // olmayan RogueSpecial'da tek bir kez bunun olması yetiyordu: aşağıdaki `contains(Recoil)` o andan sonra
        // sonsuza kadar true dönüyor, her shot sonrası silah kaydı olmayan Recoil state'ine geçiyor ve texture'ı
        // null kalıyordu. Silah görünmez oluyor, null texture'ı okuyan ilk yer (UI'daki gun frame) çöküyordu.
        const GunStateEnum animState = AnimationComp->CurrentState;
        const auto animStateIt = AnimationComp->AnimManagerDict.find(animState);

        if ((animState == GunStateEnum::Shoot || animState == GunStateEnum::Recoil) &&
            animStateIt != AnimationComp->AnimManagerDict.end() && animStateIt->second.IsFinished())
        {
            const bool hasRecoil = AnimationComp->AnimManagerDict.contains(GunStateEnum::Recoil);
            CurrentGunState = (animState == GunStateEnum::Shoot && hasRecoil)
                                  ? GunStateEnum::Recoil
                                  : GunStateEnum::Idle;
        }

        // Reload, shot gibi animation ile bitmez. Sprite sheet ne yapıyor olursa olsun ReloadSlider, ReloadTime
        // sonrasında IsReloading'i temizler; bu nedenle pose buradan bırakılmalıdır.
        // NOTE: Bu olmazsa silah, sonraki shot state'i değiştirene kadar son reload frame'ini korur. AK'de bu,
        // tilted magazine-out pose'dur.
        if (CurrentGunState == GunStateEnum::Reload && !IsReloading)
            CurrentGunState = GunStateEnum::Idle;

        // Update logic'in kalanıyla devam et
        // Origin her frame tazelenir. Initialize'da bir kez eklenseydi editörden değeri oynatmak hiçbir şey
        // yapmazdı; ok, tick eden bir sprite değil, elle author edilen bir işaretçidir.
        ArrowComp->SetOrigin(ArrowComp->BaseOrigin + ArrowComp->arrowOriginOffset);
        ArrowComp->SetPosition(this->Position + Math::RotateVector(Rotation, Scale, ArrowComp->arrowOffset));
        ArrowComp->SetRotation(this->GetDrawProperties().Rotation);
        ArrowComp->Update();

        // Gun Animation'ını update et
        AnimationComp->Update(CurrentGunState, CurrentGunState);

        // Authored noktaların ölçüldüğü referans Origin, animation'ın ilk frame'de -- Idle pose'unda -- yazdığı
        // Origin'dir. Animation Origin'i tazeledikten hemen sonra, bir kez yakalanır.
        Hands->CaptureAnchorOriginOnce(GetOrigin());
        ComputeDrawProperties();

        // Muzzle flash position ve animation'ını update et
        MuzzleFlash->Update();

        // Projectile'ları update et
        UpdateProjectiles();

        ReloadSlider->Update();
        
        Magazine->Update();
        Shells->Update();
    }

    void GunBase::Draw()
    {
        // Projectile'ları çiz
        for (const auto& proj : projectiles)
        {
            proj->Draw();
        }

        // Projectile'larla aynı nedenle IsVisible kontrolünün üzerindedir: magazine ve kovanlar silahtan
        // ayrılmıştır; hero dash yaparken ve silah gizliyken de yerde durmaya devam ederler.
        Magazine->Draw();
        Shells->Draw();

        if (!IsVisible) return; // Dash sırasında false olur; silah çizilmemeli ancak projectile'lar çizilmelidir. Önce projectile'ları, sonra visible ise silahı çizeriz.
        GameObjectBase::Draw();

        // Silahı çiz
        SpriteBatch::Draw(GetDrawProperties());

        // Arrow representation'ı çiz
        ArrowComp->Draw();

        // Muzzle flash'i çiz
        if (MuzzleFlash->IsVisible) MuzzleFlash->Draw();
        ReloadSlider->Draw();
        
        if (CollisionComp) 
            CollisionComp->Visualize(*ETG::RenderContext::Window);
        
        // Bilerek GunBase::Draw içindeki IsVisible kontrolünün dışındadır: magazine silahtan ayrılmıştır;
        // hero dash yaparken ve rifle gizliyken düşmeye devam eder.
        Magazine->Draw();
    }

    void GunBase::UpdateProjectiles()
    {
        for (auto it = projectiles.begin(); it != projectiles.end();)
        {
            (*it)->Update();
            if ((*it)->IsPendingDestroy())
            {
                UnregisterGameObject(it->get());

                // Initialize edilmiş projectile std::move ile bu container'a taşındığından
                // (`projectiles.push_back(std::move(proj));`) object'in owner'ı bu container'dır. Element'i vector'den
                // kaldırmak unique_ptr destructor'ını çağırır. unique_ptr tek owner gerektirdiğinden owner ortadan
                // kalkınca erase call sonrasında destructor otomatik olarak hemen çağrılır.
                it = projectiles.erase(it); // Erase sonrasında iterator'ı kaldırılan element'ten sonraki iterator'a ayarla
            }
            else
            {
                ++it;
            }
        }
    }

    void GunBase::PrepareShooting()
    {
        if (Timer >= FireRate)
        {
            // Firing timer'ı reset et
            Timer = 0;

            // Silah plain shot'ın nasıl göründüğünü belirtir ve modifier'ların bunu yeniden yazmasına izin verir.
            // Silaha geri yazılmak yerine local tutulur; böylece timed effect sona erdiğinde bir şeyi restore etmek gerekmez.
            // NOTE: Burada bilerek belirli bir modifier adı verilmez. Shot biçimini değiştiren yeni modifier bağımsız
            // yazılır ve bu function'a tekrar dokunulmadan loop tarafından otomatik olarak alınır.
            ShotParams shot{.ShotCount = 1, .Spread = Spread};
            for (const auto& [type, modifier] : modifierManager)
                modifier->ModifyShot(shot);

            LastShot = shot;

            // Modifier'lar kaç bullet isterse istesin, her shot group için ammo'yu yalnızca bir kez harca
            MagazineAmmo--;

            // Shot performansı buradan başlar: ateşleme kararının verildiği tek yer burasıdır, dolayısıyla silahın
            // PrepareShooting'i override edip "gerçekten ateşledim mi" diye tekrar sorması gerekmez. Ammo shot group
            // başına bir kez harcandığından burst de tek bir kick üretir.
            Hands->OnShotFired();

            // Kovan da buradan çıkar: ateşleme kararının verildiği tek yer burasıdır. Ammo shot group başına bir
            // kez harcandığından burst tek kovan atar -- her mermi için bir kovan istenirse EnqueueProjectiles'a
            // taşınmalıdır.
            Shells->Eject(*this);

            EnqueueProjectiles(shot.ShotCount, shot.Spread);
        }

        // Ammo tükenmesini işle
        if (MagazineAmmo == 0)
        {
            OnAmmoRunOut.Broadcast(true);
        }
    }

    void GunBase::EnqueueProjectiles(const int shotCount, const float EffectiveSpread)
    {
        // İlave bullet'ları delay ile queue'ya ekle (yalnızca shotCount > 1 ise kullanışlıdır)
        for (int i = 0; i < shotCount; i++)
        {
            float projectileAngle = GameObjectBase::Rotation;

            // Spread variation uygula
            if (EffectiveSpread > 0)
            {
                std::mt19937 engine(std::random_device{}());
                std::uniform_real_distribution<float> dist(-EffectiveSpread, EffectiveSpread);
                projectileAngle += dist(engine);
            }

            // Bullet'ı queue'ya ekle
            bulletQueue.push_back({i * ShotDelay, projectileAngle});
        }
    }

    void GunBase::FireBullet(float projectileAngle)
    {
        // Muzzle flash ve Shoot Animation'ı yeniden başlat
        if (MuzzleFlash->IsVisible) MuzzleFlash->Restart();
        ShootSound.play();
        
        // Animation state'i ayarla
        CurrentGunState = GunStateEnum::Shoot;
        RestartCurrentAnimStateAnimation();

        // Spawn position'ı hesapla
        const ETG::Vector2f spawnPos = ArrowComp->GetPosition();

        // Velocity'yi hesapla
        const float rad = Math::AngleToRadian(projectileAngle);
        const ETG::Vector2f direction = Math::RadianToDirection(rad);
        const ETG::Vector2f projVelocity = direction * ShotSpeed;

        // Projectile spawn et. NOTE: Projectile'ları ateşleyen silaha attach ederek spawn etmeye karar verdim.
        // Collision resolution sırasında projectile'ın owner silahını bilmem ve buradan hero projectile'ı mı yoksa
        // enemy projectile'ı mı olduğunu öğrenmem gerekiyor.
        std::unique_ptr<ProjectileBase> proj = CreateGameObjectAttached<ProjectileBase>(this,*ProjTexture, spawnPos, projVelocity, Range, projectileAngle, Damage, Force);

        // Her mermi animation'ın kendi kopyasını taşır: aynı anda havada olan iki mermi farklı frame'lerde
        // patlayabilmelidir. Kopya ucuzdur, texture zaten sheet cache'inde paylaşılır.
        if (HasProjectileImpact) proj->SetImpactAnimation(ProjectileImpact);

        proj->Update();
        projectiles.push_back(std::move(proj));
    }

    void GunBase::Reload()
    {
        // Zaten reload yapılıyorsa veya magazine doluysa tekrar başlatma
        if (IsReloading || MagazineAmmo == MagazineSize.GetInt()) return;
        
        CurrentGunState = GunStateEnum::Reload; // Animation'ı update et
        RestartCurrentAnimStateAnimation();
        IsReloading = true;
        ReloadElapsed = 0.f; // Silah performansı buna göre zamanlandığından reload ile birlikte başlar
        ReloadSound.play();
        OnAmmoRunOut.Broadcast(false); // Yeniden ammo bulunduğunu bildir
        OnReloadInvoke.Broadcast(true);
        
        // Magazine zaten doluysa veya reload çalışıyorsa GunBase::Reload işlemi iptal eder; bu nedenle düşüş
        // yalnızca reload gerçekten başladığında hazırlanır.
        if (IsReloading) 
            MagazineEjected = false;
    }

    void GunBase::RestartCurrentAnimStateAnimation()
    {
        AnimationComp->AnimManagerDict[CurrentGunState].AnimationDict[CurrentGunState].Restart();
    }

    void GunBase::SetShootSound(const std::string& soundPath)
    {
        if (!ShootSoundBuffer.loadFromFile(soundPath))
            throw std::runtime_error("Failed to load gun sound");

        ShootSound.setBuffer(ShootSoundBuffer);
        ShootSound.setVolume(ShootSoundVolume);
    }

    void GunBase::SetReloadSound(const std::string& soundPath)
    {
        if (!ReloadSoundBuffer.loadFromFile(soundPath))
            throw std::runtime_error("Failed to load Reload sound");

        ReloadSound.setBuffer(ReloadSoundBuffer);
        ReloadSound.setVolume(ReloadSoundVolume);
    }
}
