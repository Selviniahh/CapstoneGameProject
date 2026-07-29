# Animasyon Rehberi

Bu doküman iki işe yarıyor:

1. Animasyon sisteminde neyin nasıl çalıştığı — **akılda tutmak zorunda kalmayasın diye**
2. Yeni bir animasyonu nasıl ekleyeceğin — adım adım

Buradaki her madde koddan doğrulandı, tahmin yok.

---

## 1. Temel kural: Loop mu, Once mu?

Her animasyon ya **döner** ya **bir kez oynayıp son karesinde durur**. Bunu kayıt anında söylemek **zorundasın**:

```cpp
AddGunAnimationForState(GunStateEnum::Idle,  Playback::Loop, IdleAnim);
AddGunAnimationForState(GunStateEnum::Shoot, Playback::Once, ShootAnim);

AddAnimationsForState<HeroDashEnum>(HeroStateEnum::Dash, Playback::Once, dashAnims);
```

| | ne zaman | örnek |
|---|---|---|
| `Playback::Loop` | başka bir şey state'i değiştirene kadar sürer | idle, run |
| `Playback::Once` | oynar, biter, **son karesinde donar** | shoot, reload, recoil, dash, hit, death, VFX |

Parametrenin varsayılanı **yok**. Yeni animasyon eklerken derleyici sana sormaya zorluyor — sessizce döngüye düşmesi imkânsız.

---

## 2. `IsFinished()` ne demek

```cpp
bool Animation::IsFinished() const
{
    return Loops ? CurrentFrame == FrameX - 1 : HasFinished;
}
```

**`Once` ise dürüst.** Son kare tam süresini doldurunca `true` olur ve `Restart()`'a kadar `true` kalır.

**`Loop` ise "son karede mi" demek.** İki tuzağı var:
- her turda tekrar `true` olur → bu bir *olay* değil, *durum*
- son kareye varmak bir interval eksik sürer

Bu yüzden `Once` var. Döngüsel bir animasyona bunu sorman gerekiyorsa gerçekten "son karesinde mi" demek istediğinden emin ol.

### Süre formülü — bu yüzden bir gün kaybettim

```
Once:  toplam süre        = kare sayısı × interval
Loop:  son kareye varış   = (kare sayısı - 1) × interval
```

`interval` **kare başına** süredir, animasyonun toplam süresi değil. 3 kareli `0.08` bir animasyon 0.08 saniye sürmez, **0.24** saniye sürer.

---

## 3. Silahlarda zamanlama bütçesi

Bir atış döngüsünün bütçesi `FireRate`. Basılı tutarken her tick'te shoot animasyonu baştan başlar, dolayısıyla **shoot + recoil o bütçeye sığmalı** ki ikisini de tam görebilesin.

AK47 örneği (`FireRate = 0.4`, her ikisi de 3 kare):

```
3 × ShootInterval + 3 × RecoilInterval ≤ 0.4
        ShootInterval + RecoilInterval ≤ 0.133

şu an: 0.05 + 0.08 = 0.13   →  toplam 0.39s, sığıyor
```

Sığmaması hata değil — bir sonraki atış recoil'i keser, tüfek için gayet iyi durur. Ama **bilerek** taşır, kazara değil. "Basılı tutmak vs tek tek basmak" hissi tam olarak bu payla ayarlanır.

> **Uyarı:** `FireRate` `AK47.cpp`'de, interval'lar `AK47.h`'de. Aralarındaki bağı kod hiçbir yerde kurmuyor. FireRate'i değiştirirsen bu hesabı elle yeniden yap.

---

## 4. Yeni animasyon eklemek — adım adım

### a) PNG'leri koy

```
Resources/<kategori>/<isim>/<Durum>/<önek>_001.png, _002.png, ...
```

- `001`'den başla, boşluk bırakma
- **En fazla 9 kare.** `LoadFrames` sayacı tek haneli: `_009`'dan sonra `_0010` arar, bulamaz, durur
- Aynı klasörde farklı ön eklerle birden fazla animasyon durabilir (`bullet_reload_001` ve `bullet_reload_smoke_001` çakışmaz)

### b) State enum'ına ekle (yeni bir state'se)

```cpp
enum class GunStateEnum { Idle, Recoil, Shoot, Reload };
BOOST_DESCRIBE_ENUM(GunStateEnum, Idle, Recoil, Shoot, Reload)
```

### c) `SetAnimations()` içinde kaydet

```cpp
Animation RecoilAnim = {Animation::CreateSpriteSheet(
    "Guns/AK47", "ak47_shoot_recoil_001", "png", RecoilAnimInterval)};
AddGunAnimationForState(GunStateEnum::Recoil, Playback::Once, RecoilAnim);
```

Interval'ı çıplak sayı olarak yazma — `AK47.h`'deki gibi isimli bir alana koy ki tek yerden ayarlanabilsin.

### d) State geçişini yaz

`Once` bir animasyon kendi kendine state değiştirmez. `GunBase::Update`'teki örnek:

```cpp
if (const GunStateEnum animState = AnimationComp->CurrentState;
    (animState == GunStateEnum::Shoot || animState == GunStateEnum::Recoil) &&
    AnimationComp->AnimManagerDict[animState].IsFinished())
{
    const bool hasRecoil = AnimationComp->AnimManagerDict.contains(GunStateEnum::Recoil);
    CurrentGunState = (animState == GunStateEnum::Shoot && hasRecoil)
                          ? GunStateEnum::Recoil : GunStateEnum::Idle;
}
```

`contains()` ile kontrol etmek işe yarıyor: recoil animasyonu olmayan silah doğrudan idle'a döner, `GunBase`'e dokunmadan başka bir silaha recoil ekleyebilirsin.

**State değişince animasyon otomatik başa sarar** (`ChangeAnimStateIfRequired`). Elle `Restart()` çağırmana gerek yok.

---

## 5. Sprite hizalama (origin) kuralları

### Origin ilk kareden alınır ve **tüm karelere** uygulanır

`BaseAnimComp::AddAnimationsForState` origin'i ilk karenin merkezine kurar. Kareler farklı boyuttaysa her kare kayar. Stok sprite'larda bu var, hafif bir titreme yaratıyor.

**Çözüm:** yeni animasyonlarda her kareyi **aynı canvas boyutunda** üret. Titreme sıfırlanır.

### Bir state'in origin'i diğerine uymalı

Kareler sheet'e **üstten hizalı** birleştiriliyor. Bir durumdan diğerine geçerken karakter zıplamasın istiyorsan, karakteri canvas üzerinde aynı noktaya koy.

Örnek — BulletMan `CarryBody` animasyonları 34×57 canvas kullanıyor çünkü stok 12×23 gövde ortalanınca origin tam stok sprite'ların origin'ine denk geliyor.

### Silahlarda origin = **dönme pivotu**

Silah nişan alırken origin etrafında döner. Silahı 1 piksel kaydırmak için origin'e **dokunma** — hata nişan açısıyla büyür.

Doğru kaldıraç:

```cpp
HeldOffset = {0.f, -1.f};   // GunBase üyesi, Character::UpdateGuns uygular
```

Sadece silah eldeyken geçerli, yerdeki silahı etkilemez, pivot bozulmaz.

> `GunBase::OriginOffset` **ölü koddur.** `BaseAnimComp::Update` her frame `Owner->SetOrigin(animState.Origin)` yaptığı için üzerine yazılır. Kullanma.

---

## 6. VFX (MuzzleFlash sınıfı)

`MuzzleFlash` aslında muzzle flash değil — **"parent'a bağlı, tetiklenince bir kez oynayan animasyon"**. RogueSpecial'da üç örneği var: ateş alevi, reload alevi, reload dumanı.

```cpp
ReloadSmoke = CreateGameObjectAttached<class MuzzleFlash>(this);
ReloadSmoke->SetAnimation("Guns/RogueSpecial/Reload/", "RogueSpecial_reload_smoke_001", "png", 0.12f);
ReloadSmoke->SetParent(this);
ReloadSmoke->SetAttachmentOffset({13, -10});
ReloadSmoke->SetOrigin({2.f, 11.f});
ReloadSmoke->SetInheritParentRotation(false);
ReloadSmoke->Deactivate();
```

**Dikkat edilecekler:**

- **Origin'i elle ver.** `SetAnimation` origin'i *birleştirilmiş sayfanın* merkezine kurar; çok kareli şeritte bu anlamsız (8 kare × 14px / 2 = 56).
- **`SetInheritParentRotation(false)`** → dik durur ve yönünü parent'ın flip'inden alır. Duman gibi "hep yukarı çıkan" şeyler için. Alev gibi namlu yönünü takip etmesi gerekenlerde `true` (varsayılan) bırak.
- Offset ekseni: `-Y` yukarı, `+Y` aşağı, `-X` sol, `+X` sağ. Offset silahın açısıyla döner, `Scale.y < 0` iken y'si otomatik aynalanır.
- `SetAnimation` çağırmazsan VFX sessizce hiçbir şey yapmaz (`Activate()` guard'lı). Çökmez.

---

## 7. Tuzaklar listesi

Bunlar "karmaşık" değil, **tutarsız**. Bir şeyin ekranda nereye çizildiğini anlamak için 4 ayrı yere bakman gerekiyor.

| tuzak | detay |
|---|---|
| `interval` kare başınadır | toplam süre değil |
| en fazla 9 kare | `LoadFrames` sayacı tek haneli |
| origin ilk kareden gelir | tüm karelere uygulanır, boyut farkı kayma yaratır |
| `SetAnimations()` `Owner` atanmadan çalışır | `CreateGameObjectAttached` önce kurar, sonra `Owner` atar → anim comp içinden sahibine ulaşamazsın |
| `GunBase::Initialize` iki kez çalışır | biri GunBase constructor'ından, biri türetilmiş sınıftan |
| `OriginOffset` ölü | her frame üzerine yazılıyor |
| silahta sola bakış `Scale.y = -1` | `Scale.x` değil (`FlipSpritesY`) |
| `MuzzleFlash` origin'i sayfa merkezi | kare merkezi değil |
| `GunBase.h` içindeki `MuzzleFlash` **üyesi** aynı adlı **tipi** gölgeler | `std::unique_ptr<class MuzzleFlash>` yazmak zorundasın |

---

## 8. Bu sistemde ne değişti (2026-07-28/29)

### Animasyon motoru

- `Animation`'a `Loops` + `HasFinished` eklendi; `Once` animasyonlar son karede duruyor
- `IsAnimationFinished()` → **`IsFinished()`** olarak yeniden adlandırıldı
- `AddAnimationsForState` / `AddGunAnimationForState` artık zorunlu `Playback` parametresi alıyor
- 23 kayıt sınıflandırıldı: 8 `Loop`, 15 `Once`
- `PlayOnlyLastFrame` / `StopPlayingLastFrame` / `IsPlayingLastFrame` / `IsOnLastFrame` / `OriginalFrameInterval` **tamamen silindi** — ölüm animasyonları artık kendiliğinden son karede duruyor, her tick'te yeniden sabitleyen kod yok

### Silahlar

- `GunBase` artık RogueSpecial'ın muzzle flash'ını hardcode etmiyor; her silah kendi sheet'ini `Initialize`'da veriyor
- `MuzzleFlash` constructor'ı boşaltıldı, `SetAnimation()` eklendi
- `MuzzleFlash`'a `SetInheritParentRotation()` eklendi
- `GunBase::HeldOffset` eklendi — silahın eldeki duruşunu pivot'a dokunmadan kaydırmak için
- AK47'ye `Recoil` state'i ve `Shoot → Recoil → Idle` geçişi eklendi

### Yeni varlıklar

- `Resources/Guns/RogueSpecial/Reload/` — 8 kare reload + 8 kare namlu dumanı
- `Resources/Enemy/BulletMan/CarryBody/` — 64 kare (pickup / carry / carry_idle / throw), **henüz bağlanmadı**

---

## 9. Sırada ne var

`CarryBody` animasyonlarını bağlamak. İkisi de `Playback::Once`:

- **pickup** → bitince `carry`'ye geç
- **throw** → 5. karede uçan cesedi spawn et, bitince `idle`'a dön

Detaylar `Resources/Enemy/BulletMan/CarryBody/README.md` içinde.
