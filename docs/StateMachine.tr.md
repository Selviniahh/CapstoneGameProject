# Hiyerarşik State Machine

*English version: [StateMachine.md](StateMachine.md)*

Refactor sonrası hero'nun state'leri nasıl çalışıyor, neden böyle yapıldı ve düşmanları da aynı sisteme
taşımak istediğinde ne yapman gerekiyor.

---

## 1. Sorun neydi

Hero'nun state'i tek bir alandı: `HeroStateEnum CurrentHeroState`, artı isteyenin çağırabildiği bir
`SetState()`. Altı yer bunu çağırıyordu: `Hero.cpp`'deki iki health listener, `HeroMoveComp::UpdateMovement`'ın
üç dalı ve `HeroAnimComp::Update`'teki iki nokta. O frame en son kim çalıştıysa o kazanıyordu.

State'in sahibi olmadığı için öncelik, savunma amaçlı kontrollerle taklit edilmek zorundaydı.
`HeroMoveComp::UpdateMovement`, `GetState() != Die && GetState() != Hit` kontrolünü **üç dalının hepsinde**
tekrarlıyordu; aynı guard `IsDashAvailable()` içinde ve hasar listener'ında bir kez daha karşımıza çıkıyordu.
Tek bir state eklemek, bu noktaların hepsini bulup güncellemek demekti.

Animasyon bileşeni ise yavaş yavaş gameplay otoritesi haline gelmişti. `HeroAnimComp` içinde `IsDashing`,
`DashTimer` ve `MinDashDuration` duruyordu; `StartDash()` hero'nun state'ini set ediyor, `EndDash()` hareket
bileşeninin cooldown'ını başlatıyor, `Update()` ise bir vuruşun ne zaman bittiğine karar veriyordu. Dash'in aynı
anda üç ayrı doğruluk kaynağı vardı (`HeroStateEnum::Dash`, `HeroAnimComp::IsDashing`,
`HeroMoveComp::DashTimer`) — `HeroMoveComp.cpp` içindeki `HeroPtr->MoveComp->HeroPtr->AnimationComp->IsDashing`
satırı tam olarak bu yüzden vardı.

`HeroStateFlags` zaten bir hiyerarşiydi, sadece elle bitmask olarak yazılmıştı.
`CanShoot = StateIdle|StateRun` demek, "Idle ve Run aynı tür state'tir" cümlesinin uzun yoldan yazılmış hali.
Her kural iki kez yazılmak zorundaydı: bir kez `Prevent*`, bir kez `Can*` olarak.

### Yolda bulunan iki gerçek bug

**Animasyon restart'ı hiç çalışmamış.** `BaseAnimComp::Update` önce `CurrentAnimStateKey = animKey` ataması
yapıyor, *sonra* `newKey != CurrentAnimStateKey` karşılaştırması yapan `ChangeAnimStateIfRequired(animKey)`
çağırıyordu. Bu karşılaştırma her zaman false'tu, yani restart yolu ölü koddu. `StartDash` ve hit handler'ının
elle `Restart()` çağırmak zorunda kalmasının sebebi buydu. Önce `ChangeAnimStateIfRequired` çağrılıp atamanın
sahipliği ona verilerek düzeltildi.

**Flag operatörleri kısıtsızdı.** `ETG` namespace'indeki `template<typename T> T operator|(T, T)`, ADL üzerinden
o namespace'teki *her* tip için overload adayıydı. Artık `requires std::is_enum_v<T>` ile kısıtlı ve iki flag
başlığının tek tanımı paylaşması için `Managers/Enum/FlagOperators.h`'a taşındı.

---

## 2. Ağacın şekli

```
HeroRoot
├── Alive                    verir: CanTakeDamage
│   ├── Locomotion           verir: CanMove | CanShoot | CanSwitchGuns | CanUseActiveItems | CanFlipAnims
│   │   ├── Idle   (varsayılan)
│   │   └── Run
│   ├── Dash                 verir: CanFlipAnims, alır: CanTakeDamage
│   └── Hit                  alır: CanTakeDamage | CanFlipAnims
└── Dead                     her şeyi alır, hiç çıkış transition'ı tanımlamaz
    └── Die
```

Dosyalar:

| Dosya | Görevi |
|---|---|
| `Engine/Core/StateMachine/StateNode.h` | Tek bir düğüm: guard'lar, enter/exit/tick hook'ları, grants/revokes, transition'lar |
| `Engine/Core/StateMachine/HierarchicalStateMachine.h` | Generic makine. Sadece `std` ve `EventDelegate.h`'a bağımlı |
| `Game/Characters/HeroStateMachine.h/.cpp` | Hero'nun ağacı. Hero'nun yapabildiği her geçiş `Build()` içinde |
| `Game/Managers/Enum/HeroCapability.h` | İzin bitleri; `HeroStateFlags`'in yerini alıyor |
| `Game/Managers/Enum/FlagOperators.h` | Flag enum'larının paylaştığı kısıtlanmış bitwise yardımcılar |
| `Game/Characters/HeroStates.h` | Hero'nun enum'ları: `HeroStateEnum` artı animasyon anahtarları. Sadece hero include ediyor |
| `Game/Characters/HeroDirections.h/.cpp` | Yön → hero'nun animasyon anahtarları, artı dash tuşları |

**Enum'lar kaldı.** Yaprak düğümler bir `HeroStateEnum` taşıyor, dolayısıyla `AnimManagerDict`, `AnimationKey`
ve boost::describe editör UI'ı hiç dokunulmadan kaldı. Polimorfik state sınıflarına geçmek, hiçbir kazanç
olmadan bu üçünü de yeniden yazmak demekti.

---

## 3. Nasıl çalışıyor

### İki kural

1. **Transition'lar kökten yaprağa doğru değerlendirilir.** `Alive` üzerinde tanımlı bir kesme (interrupt),
   `Locomotion`'ın derinlerindeki `Idle → Run`'dan önce kontrol edilir. Öncelik buradan gelir: ne sıralama
   tablosu var, ne de `!= Die` kontrolleri. Tek bir düğümün içinde ise tanımlanma sırası belirleyicidir —
   `Alive` üzerinde bu sıra Dead, sonra Hit, sonra Dash.
2. **Çıkış transition'ı olmayan düğüm terminaldir.** `Dead` hiç transition tanımlamıyor, yani diriliş
   *engellenmiyor*, erişilemez durumda. Kaç request birikirse biriksin hiçbir şey hero'yu ayağa kaldıramaz.

### Capability'ler

`HasCapability()` aktif yolu gezer, bütün `Grants`'leri birleştirir, bütün `Revokes`'ları birleştirir ve
revoke'ları kazandırır. `Idle` ve `Run` hiçbir şey tanımlamaz — ayaktayken geçerli olan setin tamamını
`Locomotion`'dan miras alırlar. `Dash` ise `Locomotion`'ın dışında durur, dolayısıyla `CanMove`'u hiç almaz;
`CanFlipAnims`'i kendisi geri verir ve ebeveyninin dağıttığı `CanTakeDamage`'i geri alır.

`Hero::CanMove()` ve arkadaşlarının imzaları hiç değişmedi. Bütün çağıran taraflar aynı kaldı.

### Komut değil, request

Makinenin dışındaki hiçbir şey state ataması yapmaz. Input ve listener'lar tek seferlik bir niyet bildirir:

```cpp
hero.RequestDash(HeroDirections::GetDashEnum());            // InputComponent
hero.RequestHit(knockbackDir, forceMagnitude);             // hasar listener'ı
```

Bir guard bu bayrağı okur, hedef düğümün `OnEnter`'ı da onu tüketir. `InputComponent`'ın artık hero'nun zaten
dash atıp atmadığını, ölü mü yoksa vuruş animasyonunun ortasında mı olduğunu bilmesine gerek yok — makinenin
şu an geçerli bir transition'ı ya vardır ya yoktur.

**Bir request tam olarak bir tick yaşar.** `Hero::ExpireRequests()`, `Tick()`'ten hemen sonra çalışır ve kimsenin
işlemediği ne varsa düşürür. Input, tuş basılı olduğu her frame `RequestDash`'i yeniden dosyalar; yani tuşu
basılı tutmak dash'i eskiden olduğu gibi zincirlemeye devam eder — tempoyu belirleyen şey cooldown. Süre
dolmasının engellediği şey, dosyalandığı anda geçerli bir transition bulamamış (zaten dash'te, ya da cooldown
bitmemiş) bir request'in ortalıkta bekleyip yarım saniye sonra, oyuncu tuşu bırakalı çok olmuşken kendini
harcaması.

### Bir transition bir şeyi değiştirmek zorundadır

Hedefi, zaten içinde bulunduğumuz state'e çözümlenen bir transition asla alınmaz. Composite düğümler state
değildir, dolayısıyla "çözümlenmek" varsayılan çocuklara bir yaprağa varana kadar inmek demektir: `Run`'dayken
`Locomotion`'a yeniden girmek `Idle`'a iner, bu yüzden gerçek bir değişikliktir; `Dash`'in içindeyken
`Alive → Dash` ise değildir.

Bu bir mikro-optimizasyon değil, 1. kuralı güvenli kılan şeyin ta kendisi. Bir ebeveynde tanımlı transition,
bir *çocuk* aktifken de değerlendirilmeye devam eder — `Alive → Dash`'i `Alive`'a koymanın bütün amacı zaten bu —
yani hero'nun `Dash` içinde geçirdiği her frame de, input'un sürekli yeniden dosyaladığı bir request'e karşı
değerlendirilir. Onu almak ne `OnExit` ne de `OnEnter` çalıştırır (iki gezinti de en yakın ortak atada durur, ki
self transition'da bu yaprağın kendisidir) ama `TimeInState()`'i yine de sıfırlar. Sonuçta hero her frame
yeniden başlayan bir dash'in içinde kalıyordu: hiç hareket etmiyordu, çünkü `MakeDashMovement` çan eğrisini
sürekli `t = 0`'da örnekliyor ve `sin(0)` sıfır; hiç bitmiyordu da, çünkü `Dash → Locomotion` hiç büyüyemeyen bir
sayacı bekliyor. Eski kodda aynı kural `HeroAnimComp::StartDash`'in en başındaki `if (IsDashing) return;` olarak
duruyordu — kararı makineye taşırken düşen şey buydu.

### Tick başına bir karar

`Tick()` **en fazla bir yeni karar** alır, sonra ağacın içeri doğru oturmasına izin verir. Oturma geçişi yalnızca
az önce girdiği alt ağacın içinde tanımlı transition'ları dikkate alır.

Bu kısıt iki yönde de kritik:

- İçeri doğru **oturmak zorunda**: `Dash`'ten çıkmak `Locomotion`'a düşer, onun varsayılan çocuğu da `Idle`'dır;
  ama oyuncu hâlâ bir hareket tuşunu basılı tutuyorsa `Idle → Run` aynı tick içinde tetiklenmeli, yoksa hero bir
  frame boyunca yanlış animasyonu çizer.
- Dışarıdan ikinci bir karar **almamalı**: aynı anda hem bir hit hem bir dash beklerken hero `Hit`'e girip, hit
  animasyonu tek bir frame bile çizilmeden `Dash`'e geçerdi. *(Bunu kod okuyarak değil, test sürücüsü yakaladı —
  `Tick`'in ilk versiyonu tam olarak bunu yapıyordu.)*

`TimeInState()` giriş frame'i boyunca 0'dır; `Hit → Locomotion`'ın, animasyon bileşeni guard'ın beklediği
animasyonu yeniden başlatma fırsatı bulamadan tetiklenmesini engelleyen şey de budur.

### Bir frame içindeki sıra

```cpp
UpdateComponents();                         // input toplanır, kuvvetler çözülür, health tick'lenir
StateMachine->Tick(*this, Time::FrameTick); // state'e karar ver, sonra ona ait davranışı çalıştır
UpdateAnimations();                         // hangi state'e vardıysak onu çiz
```

`HeroMoveComp::Update()` hâlâ ilk fazda çalışıyor ama sadece kuvvetleri çözmek için — knockback'in her state'te
çalışmaya devam etmesi gerekiyor. Asıl yürüme (`Locomotion::OnTick`) ve dash (`Dash::OnTick`) düğüm davranışı.

---

## 4. Eski kod nereye gitti

| Eskiden | Şimdi |
|---|---|
| `HeroAnimComp::StartDash` | `Dash::OnEnter` + `HeroMoveComp::BeginDash` |
| `HeroAnimComp::EndDash` | `Dash::OnExit` |
| `HeroMoveComp::ApplyDashImpulse` + `OnDashStart`/`OnDashEnd` delegate'leri | yok; enter/exit hook'larının kendisi zaten o olay |
| İki ayrı `DashTimer` alanı | `StateMachine->TimeInState()` |
| `HeroAnimComp::Update`'teki hit-bitti bloğu | `Hit → Locomotion` transition'ı |
| `HeroAnimComp::Update`'teki ölüm dondurma bloğu | `Die::OnTick` |
| Hasar listener'ındaki knockback | `Hit::OnEnter` |
| `HealthComp->OnDeath` listener'ı | `Alive → Dead` guard'ı `IsDead()`'i doğrudan okuyor |
| `HeroAnimComp::Update`'teki `switch` | `SetKeyResolver()` kayıtları |
| 5 adet `if (state != Die && state != Hit)` | ağacın şekli |

`HeroAnimComp` artık sadece bir animasyon bileşeni: bir key çözümle, editörden ayarlanabilen frame aralığını
uygula, base'i çağır. Base çağrısından sonra hiçbir şey yok.

---

## 5. Düşmanları taşımak

Makine generic, dolayısıyla bu iş büyük ölçüde bir ağaç yazmaktan ibaret. Önerilen şekil:

```
EnemyRoot
├── Alive                    verir: CanFlipAnims
│   ├── Combat               verir: CanMove | CanShoot
│   │   ├── Idle   (varsayılan)
│   │   ├── Run
│   │   └── Shooting
│   └── Hit                  alır: CanMove | CanShoot
└── Dead                     her şeyi alır, çıkış transition'ı yok
    └── Die
```

Dikkat: düşman hero **değil**. `EnemyStateFlag::CanFlipAnims`, `StateHit`'i de içeriyor; yani burada `Hit`,
`CanFlipAnims`'i geri almamalı. Hero'nun grants'lerini kopyalamadan önce mevcut flag'leri oku.

### Adımlar

1. **`HeroCapability.h`'ın yanına `EnemyCapability.h` ekle**: `CanMove`, `CanShoot`, `CanFlipAnims`, `All`.
   Sadece pozitif taraf — `Revokes` zaten `Prevent*` yarısını karşılıyor.

2. **`EnemyStateMachine` ekle**, `HierarchicalStateMachine<EnemyStateEnum, EnemyBase, EnemyCapability>`'den
   türeyen; `BulletMan` (ve sonraki düşmanlar) ortak şeklin üstüne kendi düğümlerini ekleyebilsin diye
   `virtual void Build()` ile.

3. **Düşman başına bir makine.** Hero singleton, düşman değil. `EnemyBase` bir
   `std::unique_ptr<EnemyStateMachine>` sahiplensin, constructor'ında build edilip başlatılsın. Ağacı sakın
   static ya da paylaşımlı yapma — düğüm pointer'ları ve aktif yol örnek başına state'tir.

4. **Transition'ları teker teker içeri taşı.** Mevcut `SetState` çağrıları şöyle eşleşiyor:

   | Şimdiki çağrı yeri | Ne oluyor |
   |---|---|
   | `MoveComp->OnForceStart` → `Hit` | `MoveComp->IsBeingForced` üzerinde `Alive → Hit` guard'ı |
   | `MoveComp->OnForceEnd` → `Idle` | `!IsBeingForced` üzerinde `Hit → Combat` guard'ı |
   | `HealthComp->OnDeath` → `Die` | `HealthComp->IsDead()` üzerinde `Alive → Dead` guard'ı |
   | `EnemyMoveCompBase` → `Run` / `Idle` | `Combat` içinde, hero'ya olan mesafeye bağlı `Idle ↔ Run` |
   | `BulletMan::BulletManShoot` → `Shooting` | cooldown dolunca `Combat → Shooting` |
   | `BulletMan::UpdateShooting` → `Idle` | silah animasyonu bitince `Shooting → Combat` |
   | `BulletMan::HandleHitForce` → `Hit` | zaten `Alive → Hit` kapsıyor |

5. **`EnemyMoveCompBase::UpdateAIMovement`'taki "ateş ediyorsa state değiştirme" guard'ını sil.** `Shooting`,
   `Combat` altında `Idle`/`Run`'ın kardeşi; yani ateş ederken `Idle ↔ Run` transition'ları zaten aktif yolda
   değil. Bu işi yapmanın bütün sebebi de bu.

6. **Ölüm yan etkilerini `Dead::OnEnter`'a taşı**: depth değişimi, knockback, delegate'lerin temizlenmesi ve
   collision'ın kapatılması. `Dead` terminal olduğu için `OnForceStart`/`OnForceEnd` temizliği artık isteğe
   bağlı, ama kuvvet sisteminin sessiz kalmasını istiyorsan bırak.

7. **`BulletManAnimComp::Update`'in switch'ini `SetKeyResolver` kayıtlarıyla değiştir**, tam olarak
   `HeroAnimComp::SetKeyResolvers`'ın yaptığı gibi. `BulletManDirections::Get*Enum` yardımcıları aynı
   kalıyor; sadece case etiketlerinden çağrılmak yerine bağlanıyorlar.

8. **Guard'lar `const EnemyBase&` alır.** BulletMan'e özgü bir şey için guard'ın içinde `owner.As<BulletMan>()`
   kullan, ya da o transition'ı tipin bilindiği `BulletMan`'in kendi `Build()` override'ından kaydet.

9. **Sonra `StateFlags.h`'tan `EnemyStateFlag`'i sil**, bu da dosyayı boş ve silinebilir bırakıyor.

### Testi de aynı şekilde yap

`docs/` altında bir test harness yok, ama hero için kullanılan iki tek kullanımlık sürücüyü yeniden yazmaya
değer:

- **Sahte bir owner struct'ı** ile, `HierarchicalStateMachine.h` dışında hiçbir şeye derlenen standalone bir
  sürücü; transition sırasını ve enter/exit dizisini assert etmek için. Tek tick'te iki karar hatasını yakalayan
  buydu.
- `libetgcore`'a link edilen, `Build()` çağırıp düğüm grafiği üzerinde assert eden bir sürücü — ebeveynler,
  varsayılan çocuklar, grants/revokes, transition hedefleri ve `Dead`'in çıkışının olmadığı. `Build()`'in ne
  pencereye ne de asset'e ihtiyacı var, headless çalışır.

---

## 6. Pratik kurallar

- **Asla bir `SetState` ekleme.** Bir şeyin karakteri bir state'e itmesi gerekiyorsa, request dosyalar ve bir
  guard onu alır. Aksi halde yine "en son kim çalıştıysa o kazanır" noktasına dönersin.
- **Kuralı, sahibi olan düğüme koy.** İki kardeş state bir izni ya da bir transition'ı paylaşıyorsa, o şey
  ebeveynlerine aittir. Kendini aynı guard'ı iki kardeşe yazarken bulduysan, eksik bir composite düğüm buldun
  demektir.
- **Kontrol yerine şekli tercih et.** "Y sırasında X olamaz" cümlesi genelde, X'in transition'ının Y'nin aktif
  yolda olmadığı bir yerde yaşaması gerektiği anlamına gelir.
- **Tek seferlik request'ler `OnEnter`'da tüketilmeli, tick'ten sonra da süresi dolmalı.** Set olarak kalan bir
  bayrak, transition'ını dosyalandığı sırada kimsenin düşünmediği bir tick'te yeniden tetikler.
- **Guard mutasyon yapmamalı.** `const OwnerT&` almasının sebebi bu. Yan etkiler `OnEnter`/`OnTick`/`OnExit`'e
  ait.
- **Frame sırasına dikkat et.** Guard'lar `UpdateAnimations()`'tan önce çalışır, yani bir animasyona soru soran
  her şey bir önceki frame'in cevabını okur. `HeroMoveComp::GetDashDuration`'ın "şu anki animasyon" diye sormak
  yerine animasyonu state ve yöne göre araması tam olarak bu yüzden.

---

## 7. Henüz yapılmadı

- `GunStateEnum` (Idle / Shoot / Reload) hâlâ düz bir enum. Bir UI nesnesi olan `ReloadSlider::FinishAnimation`,
  `Gun->CurrentGunState`'e atama yapıyor — bu da hero'nun az önce kurtulduğu katman ihlalinin aynısı.
- `Hero::MouseAngle`, `CurrentDirection` ve `IsShooting` `static`. Tek bir hero olduğu için çalışıyorlar ama
  global değiştirilebilir state'ler ve makinenin guard'ları onları okuyor.
- `AnimationKey` artık type-erased, yani yeni bir yön enum'unu merkezi bir listeye eklemek gerekmiyor. Geriye
  kalan şey, her key'in içinde bir `std::string Name` taşıması ve bu string'in hem `operator==`'e hem hash'e
  katılması — yani her animasyon aramasında bir string hash'leniyor.
