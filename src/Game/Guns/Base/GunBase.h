#pragma once
#include <memory>
#include <vector>
#include "../../../Engine/Platform/Platform.h"
#include "GunBase.h"
#include <boost/describe.hpp>
#include "../../../Engine/Platform/Platform.h"
#include "../../../Engine/Platform/Platform.h"
#include "../../../Engine/Animation/Animation.h"
#include "../../../Engine/Core/GameObjectBase.h"
#include "../../../Engine/Core/Components/ArrowComp.h"
#include "../GunStates.h"
#include "../../../Engine/Core/Components/BaseAnimComp.h"
#include "../../../Engine/Core/Events/EventDelegate.h"
#include "../VFX/MuzzleFlash.h"
#include "../../Modifiers/ModifierManager.h"
#include "../../Modifiers/Gun/IGunModifier.h"
#include "../../../Engine/Core/Stats/StatModifier.h"

namespace ETG
{
    class MagazineDrop;
    class CollisionComponent;
    class ReloadSlider;
    class ProjectileBase;
    struct QueuedBullet;
    enum class GunStateEnum;

    class GunBase : public GameObjectBase
    {
    public:
        GunBase(ETG::Vector2f Position, float fireRate, float shotSpeed, float range, float timerForVelocity, float depth, int ammoSize, int magazineSize, float reloadTime,
                float damage = 1.0f, float force = 1.0f, float forceDuration = 1.0f, float spread = 0.0f);

        ~GunBase() override;
        void Initialize() override;
        void Update() override;
        void Draw() override;

        // Hero'dan left click geldiğinde çağrılır. Bullet'ları ateşlemek için timer ve FireRate'e göre çalışır.
        // NOTE: Tüm base gun'ların yapması gereken işlemleri yürütür: ateş etmenin mümkün olup olmadığını kontrol etme,
        // MagazineAmmo azaltma, modifier uygulama, event broadcast etme vb. SHOOTING LOGIC İÇERMEZ.
        virtual void PrepareShooting(); // Bullet'ları bulletQueue içine ekle

        // NOTE: Asıl projectile firing logic budur. Custom shooting logic için child class'ta base'i çağırmadan override et
        virtual void EnqueueProjectiles(int shotCount, float EffectiveSpread);
        void RestartCurrentAnimStateAnimation();

        // NOTE: EnqueueProjectiles projectile'ları queue'ya ekler; bu function onları tick içinde ateşler
        void UpdateProjectiles(); // Gerekirse projectile'ı kaldır ve update et

        // `source` tarafından silahın herhangi bir stat'ına eklenen her şeyi detach eder. Item'ın hangi stat'lara
        // dokunduğunu hatırlaması gerekmez; böylece "item'ı bırakmak", "item'ı almak" işleminin tam tersi olur.
        void RemoveAllModifiersFrom(const std::string& source);

        virtual void Reload();
        void SetShootSound(const std::string& soundPath);
        void SetReloadSound(const std::string& soundPath);
        void FireBullet(float projectileAngle); // Tek bir bullet ateşle
        [[nodiscard]] bool IsMagazineEmpty() const { return MagazineAmmo == 0; }; // Magazine'in boş olup olmadığını kontrol et

        ModifierManager<IGunModifier> modifierManager;

        // Modifier'lar uygulandıktan sonra son shot'ın aldığı biçim. Silahın buna neden olan modifier'ı bilmeden
        // gerçekten ateşlediği şeye tepki vermesini sağlar; örneğin burst daha hızlı muzzle flash gerektirir.
        // Böylece burst üreten ikinci bir modifier da ek işlem gerektirmeden desteklenir.
        ShotParams LastShot{};
        
        // Her reload için spawn etmek yerine aynı object yeniden kullanılır. İlk reload bitmeden ikincisi
        // başlayamayacağı için havada hiçbir zaman birden fazla magazine bulunmaz.
        std::unique_ptr<MagazineDrop> Magazine;

        std::vector<QueuedBullet> bulletQueue; // Ateşlenmeyi bekleyen bullet queue'su

        // NOTE: Aynı hatayı dördüncü kez yaptığım için yazıyorum. Bu delay yalnızca active item'ın double shot
        // özelliğiyle ilgilidir; active edilmediği sürece kullanılmaz. Best practice olarak Active item'a taşınmalıdır.
        float ShotDelay = 0.1f;

        // Bu silahın artwork'ünün sabit eller altında Hand::GunOffset'e ek olarak nasıl durduğunu belirler.
        // Değeri sağ elde tutulan artwork için author et; silah taraf değiştirdiğinde Character::UpdateGuns X'i
        // mirror eder. Yalnızca silah tutulurken etkili olduğundan yerde duran silah etkilenmez.
        // NOTE: Silahı kaydırmak için Origin'i KULLANMA. Origin, nişan alırken silahın etrafında döndüğü pivot'tur;
        // kaydırılması, aim angle arttıkça büyüyen miktarda silahı hedeften saptırır. Ayrıca BaseAnimComp::Update
        // içinde her frame animation tarafından yeniden yazılır; aşağıdaki OriginOffset'in etkisiz olmasının nedeni budur.
        ETG::Vector2f HeldOffset{0.f, 0.f};

        // Silahın ellerine dair ne varsa buradadır: anchor'lar, bu frame'in gesture'ları, grip pinning tercihi ve
        // nefes / shot kick / reload reach gibi hazır hareketler. Silah bunu kendi Initialize'ında author eder;
        // çalıştırmasını GunBase::Update üstlenir.
        std::unique_ptr<class HandRig> Hands;

        // Her shot'ta dışarı atılan ve yerde kalan kovanlar. Silah yalnızca ejection port'unu ve itmeyi author
        // eder; fırlatmayı GunBase, düşüşü ve birikmeyi ejector'ın kendisi yürütür.
        std::unique_ptr<class ShellEjector> Shells;

        // Origin-relative gun-local bir noktanın şu an world içindeki konumunu verir. Böylece el veya clip'in
        // düştüğü magazine well gibi silahın bir bölümünü bulması gereken her yer rotate-mirror-unslide
        // işlemlerini elle tekrarlamak yerine burayı kullanır. Nokta zaten Origin'e göre olduğundan mevcut
        // animation frame'inin Origin değeri burada tekrar çıkarılmaz.
        [[nodiscard]] ETG::Vector2f WorldPointOnGun(const ETG::Vector2f& localPoint) const;

        // Silah üzerinde author edilen her nokta gun-local uzaydadır. Editor'ün Visualize kutucuğu bunu okuyarak
        // marker'ı gerçekte kullanıldığı yere çizer; tek bu override, hem anchor'ları hem her silahın kendi reload
        // noktalarını doğru yere oturtur. Silaha attach olan component'ler (HandRig, ShellEjector, MuzzleFlash)
        // kendi noktalarını aynı uzayda yazdığından, GameObjectBase'in owner'a devretme kuralı onları da buraya
        // getirir; o sınıfların tek satır yazmasına gerek kalmaz.
        [[nodiscard]] ETG::Vector2f ResolveDebugPoint(const char* label, const ETG::Vector2f& point) const override
        {
            // Bu ikisi silahın üzerinde bir yer değil, artwork'ün pivot'unu tarif eder: OriginOffset pivot'u
            // kaydırır ve sprite ters yöne gider, Origin ise zaten Position'a oturan pixel'dir.
            if (DebugLabelIs(label, "OriginOffset")) return OriginShiftDebugPoint(point);
            if (DebugLabelIs(label, "Origin")) return GameObjectBase::ResolveDebugPoint(label, point);

            return WorldPointOnGun(point);
        }

        // Silah solda tutulurken X'i mirror edilmiş HeldOffset değeridir. Böylece sağ eldeki artwork'e göre author
        // edilen bir değer iki tarafta da "barrel boyunca daha geride" anlamını korur.
        [[nodiscard]] ETG::Vector2f MirroredHeldOffset() const;

        // Silahın body'nin hangi tarafında tutulduğunu ve dolayısıyla sprite'ın hangi yönde mirror edileceğini belirler.
        // Doğrudan sağ yönünden ölçülen degree cinsinden half angle'dır. Aim doğrudan sağ yönünün +-HandSwapAngle
        // aralığındayken silah sağ elde kalır, bu aralığın dışında el değiştirir. Doğru değer 90'dır; barrel tam olarak
        // dikey konumu geçtiğinde mirror edilir.
        //
        // NOTE: Negatif değer, silahın bir tercihinin olmadığını belirtir ve character eskiden tüm silahlarda olduğu
        // gibi 8-way facing'e geri döner. DownRight arc 67.5 derecede bittiği için dönüş de orada gerçekleşir. Bu,
        // silah hâlâ sağa nişan alırken mirror edildiği 22.5 derecelik bir aralık bırakır.
        float HandSwapAngle{-1.f};

        [[nodiscard]] bool DecidesOwnHandSide() const { return HandSwapAngle >= 0.f; }

        // Bu aim için silahın sağ elde olup olmadığını belirtir. `aimAngle` [0,360) aralığındadır; 0 doğrudan sağı
        // gösterir ve değer clockwise artar. DirectionUtils ile aynı convention kullanılır. Yalnızca silah kendi
        // tarafına karar veriyorsa anlamlıdır.
        [[nodiscard]] bool IsHeldOnRightSide(float aimAngle) const;

        // Holder'ın bu frame'de silahın hangi elde olduğuna dair kararını içerir. Herhangi bir yer okumadan önce
        // Character::PublishHeldSideToGun tarafından yazılır. Yerdeki silah son değerini korur; bu değeri kimse okumaz.
        //
        // NOTE: Silahın bunu hesaplayacak HandSwapAngle değeri olsa da sonucu kendi başına belirlemez. Angle'ı negatif
        // bırakan silahın tercihi yoktur ve cevap silahın göremediği *body facing* olur. Sonucun bildirilmesi, her
        // silahın holder ile aynı kararı kullanmasını sağlar.
        bool IsHeldOnRightHand{true};

        // <---------- Grip pinning ---------->
        // Silahın pin isteyip istemediği HandRig::PinsGripWhenAimingUp üzerinde authored'dır. Aşağıdaki iki rule
        // ise gun state okur (barrel yukarıda mı, pin hangi angle'da donuyor), bu yüzden burada virtual kalır.
        //
        // NOTE: Pin off hand üzerinde değil, silah üzerinde olmalıdır. Yalnızca eli pin etmek, silah altından dönmeye
        // devam ederken eli havada dondururdu; birbiriyle uyuşması gereken iki lock oluşurdu. Eller daha sonra silahın
        // üzerine yerleştirildiğinden silah üzerindeki tek lock, onu tutan her şeyi de lock eder.

        // Barrel'ın iki taraftan birinde yatayın üzerine bakıp bakmadığını belirtir. Bu, holder'ın arkadan çizildiği
        // circle yarısı ve pin edilmemiş grip'in sprite dışına çıktığı tek yarıdır.
        [[nodiscard]] bool IsBarrelAboveHorizontal() const { return GetRotation() > 180.f; }

        // Yalnızca pin'i farklı bir rule izleyen silah için override et; örneğin kalıcı olarak bağlı bir boss arm.
        //
        // NOTE: Bilerek yalnızca aim'e bağlı pure function'dır. Önceden barrel yukarı çıktığında devreye giren ve yalnızca
        // silah el değiştirdiğinde bırakılan bir latch idi. Tek rule'un iki yarısı için iki farklı angle kullanılması,
        // grip'i barrel ile serbestçe yükselmesi gereken bütün bir quadrant boyunca donmuş tutuyordu.
        //
        // NOTE: Rig'in authored tercihini okuduğu için body GunBase.cpp'dedir; HandRig burada yalnızca forward
        // declare edilmiştir ve her silahı onun tam tanımını include etmeye zorlamanın anlamı yok.
        [[nodiscard]] virtual bool WantsGripPinned() const;

        // Pin edilmiş grip'in dondurulduğu angle: silahın tutulduğu tarafın horizontal pose'u. Tarafın kendi yatayını
        // seçmek pin'in zero displacement ile devreye girmesini sağlar; barrel yukarı çıkarken tam bu angle'dan geçer,
        // dolayısıyla pin devreye girdiğinde hiçbir şey sıçramaz.
        [[nodiscard]] virtual float PinnedGripRotation() const { return IsHeldOnRightHand ? 0.f : 180.f; }

        // Silah tutulurken holder body'nin önünde ve arkasında hangi depth'te çizileceğini belirler. SpriteBatch büyük
        // depth değerlerini önce sort eder; dolayısıyla ikisinden büyük olan arkadadır. Her ikisi de silahın construct
        // edildiği depth ile başlatılır. Böylece özel değer vermeyen silah her zamanki yerde çizilir; holder arkasında
        // kaybolması gereken silah HeldDepthBehindBody için kendi değerini verir.
        //
        // NOTE: Character üzerinde ortak bir "behind" değeri kullanılamaz; çünkü silahların front konumu aynı değildir.
        // RogueSpecial depth 3 ile author edildiğinden depth -1 olan hero'nun zaten arkasındadır; AK ise -2 ile önündedir.
        // Behind, yalnızca silahın bildiği bir değere göre relative'dir.
        float HeldDepthInFront{};
        float HeldDepthBehindBody{};

        float ForceDuration{1};

        using GameObjectBase::Rotation; // Rotation'ı GunBase içinde public yap
        using GameObjectBase::Depth; // Holder bunu yukarıdaki iki değerden her frame yeniden yazar

        // State direction içermez; Idle, Shoot, Reload vb. değerlerden oluşur
        GunStateEnum CurrentGunState{GunStateEnum::Idle};

        bool IsReloading{};
        
        // Mevcut reload'un magazine'i fırlatıp fırlatmadığını belirtir. Reload() içinde temizlenir; böylece GunBase'in
        // reddettiği reload (dolu magazine veya zaten çalışan reload) ikinci bir düşüşü hazırlayamaz.
        bool MagazineEjected{true};

        // Mevcut reload başladıktan sonra geçen saniye ve aynı değerin ReloadTime'ın 0..1 fraction'ı olarak ifadesidir.
        // Reload'a göre zamanlanması gereken her şey (hand gesture veya düşen magazine gibi) kendi timer'ını çalıştırmak
        // yerine bu fraction'ı okur. Aksi hâlde kendi timer'ı ReloadSlider'ın geri saydığı timer'dan sapar ve performansı
        // reload'dan farklı bir anda bitirirdi.
        float ReloadElapsed{};
        [[nodiscard]] float ReloadProgress() const;

        // Gun stat'ları. Her biri kendi base value'sunu ve item'ların eklediği modifier'ları taşır.
        //
        // NOTE: Önceden her stat için constructor içinde elle sync edilen iki field vardı: `BaseFireRate` ile `FireRate`
        // gibi. Bu ikili, FireRate'e atama yapan item'ın modifier uygulanmamış değerin tek copy'sini yok etmesi nedeniyle
        // gerekliydi; değeri geri okuyabileceği bir yer olmalıydı. PlatinumBullets'ın her Update'te tüm stat'ı base
        // üzerinden yeniden hesaplamasının nedeni de buydu. Stat iki parçayı da tutar ve item doğrudan atama yapmaz;
        // kendi adıyla modifier attach eder ve aynı adla detach eder.
        StatModifier FireRate; // Shot'lar arasındaki süre (saniye)
        StatModifier ShotSpeed; // Bullet'ların hareket hızı
        StatModifier Range; // Bullet'ların gidebildiği mesafe
        StatModifier ReloadTime; // Reload süresi
        StatModifier Damage; // Bullet başına damage
        StatModifier Force; // Enemy'lere uygulanan knockback
        StatModifier Spread; // Degree cinsinden bullet spread angle (0 = tam isabet)

        StatModifier MagazineSize; // Magazine başına bullet sayısı

        // NOTE: Bu ikisi stat değil counter'dır; bu nedenle plain int olarak kalır. MagazineAmmo açıkça counter'dır.
        // MaxAmmo capacity gibi görünse de ReloadSlider onu harcar (`MaxAmmo -= ...`); gerçekte reserve pool'dur.
        // Adı yanıltıcıdır ve onu değiştiren item capacity'yi değil, player'ın kalan bullet sayısını değiştirir.
        int MaxAmmo{};
        int MagazineAmmo{}; // Mevcut magazine ammo sayısı (azaltılır ve reset edilir)

        std::shared_ptr<ETG::Texture> ProjTexture;
        std::unique_ptr<ReloadSlider> ReloadSlider;
        EventDelegate<bool> OnAmmoRunOut;
        EventDelegate<bool> OnReloadInvoke;

    protected:
        float Timer; // Tick'e göre artar

        // Offset vector'ünü silahın mevcut rotation değerine göre döndürür
        std::vector<std::unique_ptr<ProjectileBase>> projectiles;
        std::unique_ptr<ArrowComp> ArrowComp;
        std::unique_ptr<MuzzleFlash> MuzzleFlash;

        // Silahın Hero'nun eline attach edilmesi gerektiği için custom Origin offset'e ihtiyacı vardır
        ETG::Vector2f OriginOffset;

        // Gun Animation
        std::unique_ptr<BaseAnimComp<GunStateEnum>> AnimationComp;
        
        std::unique_ptr<CollisionComponent> CollisionComp;
        

    private:
        // Sound'lar
        ETG::SoundBuffer ShootSoundBuffer;
        ETG::Sound ShootSound;

        ETG::SoundBuffer ReloadSoundBuffer;
        ETG::Sound ReloadSound;

        float ShootSoundVolume = 10;
        float ReloadSoundVolume = 10;

        BOOST_DESCRIBE_CLASS(GunBase, (GameObjectBase),
                             (CurrentGunState, MaxAmmo, MagazineSize, MagazineAmmo, ShotDelay, ReloadTime, IsReloading,
                                 FireRate, ShotSpeed, Range, Damage, Force, ForceDuration, Spread, HeldOffset,
                                 HandSwapAngle, HeldDepthInFront, HeldDepthBehindBody),
                             (ProjTexture, OriginOffset),
                             ())
    };

    // Şimdilik bullet yalnızca ateşlenme süresine ve angle değerine sahiptir
    struct QueuedBullet
    {
        float timeToFire;
        float angle;
    };
}
