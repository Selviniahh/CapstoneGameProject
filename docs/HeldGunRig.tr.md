# Tutulan Silah Düzeni (Held-Gun Rig)

Bir karakterin elindeki silahın ve o silahı tutan iki elin nerede duracağına karar veren sistem.

Tamamı `Character` üzerinde yaşıyor. Yani **Hero, BulletMan ve ileride yazılacak her boss aynı geometriyi
paylaşıyor.** Yeni bir ateş eden düşman yazarken bu dosyadaki hiçbir şeyi tekrar yazmaya gerek yok; sadece
`Character`'dan türetip aşağıdaki üç fonksiyonu tick'inde çağırmak yeterli.

---

## 1. Zincir: mouse/hedef → çizilen piksel

```
AimAngle (0..360, 0 = sağ, saat yönünde artar)
   │
   ├─► CurrentDir  (8 yönlü Direction, DirectionUtils'in yay tablosundan)
   │        │
   │        ├─► gövde sprite'ı (hangi animasyon + FlipSpritesX)
   │        └─► IsFacingBack()  →  el ve silah depth'i
   │
   └─► Character::IsGunOnRightSide()   ◄── TEK KARAR NOKTASI
              │
              ├─► HoldPoint hangi tarafta
              ├─► silah sprite'ı aynalanıyor mu (Scale.y)
              ├─► HeldOffset'in X'i aynalanıyor mu
              ├─► boş el gövdenin hangi yanında dinleniyor
              └─► GunBase::IsHeldOnRightHand (silaha bildirilir)
```

`IsGunOnRightSide()`'ın tek bir fonksiyon olması bu sistemin en önemli özelliği. Eskiden tutuş noktası, ayna ve
eller bu kararı ayrı ayrı hesaplıyordu; sonuç olarak kendi `HandSwapAngle`'ı olan bir silahın sprite'ı, onu tutan
elden **22.5 derece farklı açıda** dönüyordu. Yeni bir karar eklerken bu fonksiyonu çağır, kendin hesaplama.

---

## 2. Çağrı sırası

Karakterin tick'inden **bu sırayla** çağrılır. Sıra bir stil tercihi değil: her adım bir öncekinin yazdığı değeri
okur.

```cpp
UpdateHoldPoint();             // 1. silahın asıldığı eklem
UpdateGuns();                  // 2. silah eklemin üstüne, sonra eller silahın üstüne
UpdateHandAndGunVisibility();  // 3. bunların hiçbiri çiziliyor mu
```

`Hero::Update` ve `BulletMan::Update` ikisi de bunu yapıyor. Neden `Character::Update` içinde değil: iki dal bunu
kendi state machine'lerine göre tick'in farklı yerinde çalıştırıyor, anlaşamadıkları kısım orası.

### `UpdateGuns()` içindeki adımlar

```cpp
PublishHeldSideToGun();  // silah hangi elde olduğunu öğrenir
PlaceHeldGun();          // pozisyon + rotasyon
UpdateHeldGunDepth();    // gövdenin önü / arkası
MirrorHeldGun();         // Scale.y
ApplyGripPin();          // kabza etrafında döndürme
TickEquippedGuns();      // her silah tick'lenir
UpdateHands();           // eller silahın üstüne
```

### Kritik iki sıralama kuralı

**a) Depth ve ayna, silahın `Update()`'inden ÖNCE yazılmalı.**
`GameObjectBase::Update` → `ComputeDrawProperties()` içinde `DrawProps.Depth = Depth` satırı var. Yani depth o an
draw property'lere pişiyor. Sonra yazarsan bir kare gecikir — ve o bir kare, tam yön değiştirdiğin karedir.
Aynısı eller için de geçerli: `UpdateHandDepths()`, `PlaceHand()`'in içindeki `hand.Update()`'ten önce çağrılıyor.

**b) Eller, silahların `Update()`'inden SONRA yerleştirilmeli.**
Bir silahın `Origin`'i her karede animasyonundan yeniden yazılıyor (`BaseAnimComp::Update`). Kabza çapaları o
`Origin`'e göre ölçüldüğü için, eller taze `Origin`'i beklemek zorunda. `UpdateHands()` bu yüzden
`TickEquippedGuns()`'dan sonra.

---

## 3. Silah başına ayarlar (`GunBase`)

Hepsi `BOOST_DESCRIBE_CLASS` listesinde, yani editör panelinden canlı oynanabilir.

| Alan | Ne yapar | Boş bırakılırsa |
|---|---|---|
| `HeldOffset` | Silah artwork'ünü sabit ellerin altında kaydırır. Sağ el hâli için yazılır, sol elde X'i otomatik aynalanır | `{0,0}` — kayma yok |
| `RightHandAnchor` / `LeftHandAnchor` | Her elin silahı kavradığı piksel. Frame'in sol-üst köşesi `(0,0)` — resim editöründen okuduğun sayı | çapa yok |
| `HasRightHandAnchor` / `HasLeftHandAnchor` | Çapa gerçekten ölçüldü mü | `false` — el gövdede dinlenir |
| `HandSwapAngle` | Silahın el değiştirdiği yarı-açı (sağdan itibaren derece). `90` = namlu dikeyi geçince | `-1` — gövdenin 8 yönlü facing'i karar verir (67.5°'de döner) |
| `PinsGripWhenAimingUp` | Namlu yataydan yukarı çıkınca ön kabzayı gövdeye çiviler | `false` — kabza serbest |
| `HeldDepthInFront` / `HeldDepthBehindBody` | Tutulurken gövdenin önünde / arkasında hangi depth | ikisi de ctor'daki `depth` — davranış değişmez |
| `IsHeldOnRightHand` | *Okunur, yazılmaz.* Sahibi her kare yazar | — |

### Neden `HandSwapAngle` varsayılanı `-1`

`DirectionUtils`'in yay tablosunda `DownRight` yayı 67.5°'de bitiyor, ve `IsFacingRight` orada taraf değiştiriyor.
Yani gövdenin facing'i 67.5°'de dönüyor. Silah sprite'ı için doğru değer **90**'dır (namlu tam dikeye geldiği an).
Arada kalan 22.5°'lik bantta silah, hâlâ sağa nişan alırken aynalanmış — yani ters — görünür. `-1` eski davranışı
koruduğu için varsayılan; yeni silah yazarken `90` ver.

---

## 4. Kabza çivilemesi (grip pin) — mekanizma

**Problem.** Silah kendi `Origin`'i etrafında dönüyor. AK47'nin `Origin`'i frame'in merkezi, kabzası ise merkezin
6.5 piksel solunda. Namlu yukarı bakınca kabza aşağı savruluyor ve karakterin sprite'ının **dışında** kalıyor.
Orijinal Enter the Gungeon'da sırt animasyonunda kabza gövdeye yapışık duruyor.

**Çözüm.** Silahı, kabza pikseli sabit bir noktaya gelecek şekilde kaydır. Silah artık `Origin`'i etrafında değil
**kabzası etrafında** döner:

```cpp
gripLocal      = LeftHandAnchor - Origin                        // silah uzayında kabza
whereTheGripIs = Rotate(GetRotation(),        scale, gripLocal)  // şu anki yeri
whereItStays   = Rotate(PinnedGripRotation(), scale, gripLocal)  // durması gereken yer
Position      += whereItStays - whereTheGripIs
```

**Kilit silahın üstünde, elin üstünde değil.** Bu can alıcı nokta. Kilit ele uygulandığında el havada donuyor,
silah altından dönmeye devam ediyordu — birbirini tanımayan iki kilit. Eller silahtan *sonra* yerleştirildiği
için (`UpdateHands`), silahın üstündeki tek kilit onu tutan her şeyi kilitler. İkisinin ayrılması artık
matematiksel olarak imkânsız.

**Referans açı taraf başına.** `PinnedGripRotation()` sağ elde `0`, sol elde `180` döndürür — yani o tarafın
kendi yatay pozu. Namlu yukarı çıkarken tam o açıdan geçtiği için kilit **sıfır kaymayla** devreye girer,
kurulma anında sıçrama olmaz.

### Latch kullanma

`WantsGripPinned()` nişan açısının **saf bir fonksiyonu**: `PinsGripWhenAimingUp && GetRotation() > 180`.

Bu bir zamanlar latch'ti: namlu yukarı çıkınca kurulur, silah el değiştirince bırakılırdı. Kurulma ve bırakma
açılarının farklı olması, kabzanın serbest olması gereken koca bir çeyrekte (0°→90°, yani namlu aşağı inerken)
donmuş kalmasına yol açtı. Orijinal oyunda kabza orada namluyla birlikte yukarı kalkar.

**Ders:** bir kuralın kurulma ve bırakma koşulu farklıysa, o kural yanlış yerde duruyor demektir. Durum tutmak
zorunda kalıyorsan, önce kuralı açının saf bir fonksiyonu olarak yazmayı dene.

---

## 5. Depth: hangi sayı önde?

`SpriteBatch::end` şöyle sıralıyor:

```cpp
if (a.depth == b.depth) return a.drawOrder < b.drawOrder;
return a.depth > b.depth;
```

**Büyük depth önce çizilir → arkada kalır.** Yani sayı büyüdükçe geriye gidiyorsun. Sezgiye ters, unutma.

Sırt animasyonlarında (`IsFacingBack`: `Up`, `UpLeft`, `UpRight`) mevcut düzen, arkadan öne:

```
AK47 (1)  →  eller (0)  →  hero (-1)
```

İkisi de gövdenin arkasında, ama el silahın önünde — namlu kabzayı tutan parmakları kapatmasın diye.

Karakter üzerinde tek bir "arka" değeri işe yaramaz, çünkü silahlar "ön"de anlaşmıyor: RogueSpecial `3`'te
(hero `-1`'in zaten arkasında), AK47 `-2`'de (önünde). Arka, yalnızca silahın bildiği bir sayıya göreli. O yüzden
iki değer de `GunBase`'de.

---

## 6. Checklist: yeni silah eklerken

`Initialize()` içinde, `GunBase::Initialize()` çağrısının etrafında:

1. `HandSwapAngle = 90.f;` — neredeyse her zaman doğru olan değer.
2. Çapaları resim editöründen oku. Frame'in sol-üst köşesi `(0,0)`. Tetik elini `RightHandAnchor`'a,
   varsa ön kabzayı `LeftHandAnchor`'a yaz, `Has...` bayraklarını aç.
3. İki elle tutuluyorsa `PinsGripWhenAimingUp = true;`.
4. Sırt animasyonunda gövdenin arkasına geçmesi gerekiyorsa `HeldDepthBehindBody`'ye bir sayı ver
   (hero için `1.f` çalışıyor). Tek elli/kısa silahlarda gerekmeyebilir.
5. Sprite ellerin altında kaymışsa `HeldOffset` ile düzelt — **`Origin`'e dokunma.** `Origin` silahın nişan
   alırken döndüğü pivot; kaydırırsan silah hedefin dışına kayar ve kayma miktarı nişan açısıyla büyür. Üstelik
   `BaseAnimComp::Update` onu her kare animasyondan yeniden yazar.

## 7. Checklist: ateş eden yeni düşman/boss eklerken

1. `Character`'dan türet (`EnemyBase` zaten türüyor).
2. `Hand` ve `OffHand`'i oluştur (`CreateGameObjectAttached<class Hand>(this)`).
3. Sanatına göre `HandOffsetRight` / `HandOffsetLeft` ver. Hero `{8,5}` / `{-7,5}` kullanıyor.
4. `AimAngle` ve `CurrentDir`'i her kare doldur. Hero mouse'tan, BulletMan hedefine olan açıdan alıyor.
5. Tick'inde sırayla: `UpdateHoldPoint()` → `UpdateGuns()` → `UpdateHandAndGunVisibility()`.
6. `ShouldShowHeldGun()`'ı gerekiyorsa override et (hero dash ve hit sırasında saklıyor, BulletMan sadece ölünce).
7. `Draw()` içinde silahları ellerden **önce** submit et; aynı depth'te çizim sırası belirleyici.

Geometrinin tamamı bedavaya gelir. Kabza çivilemesi, el değiştirme, aynalama, depth — hiçbiri tekrar yazılmaz.

---

## 8. Bilinen pürüzler

- **270°'de küçük bir sıçrama.** Tam yukarıda el değişimi, ayna, kilidin referansı ve `HoldPoint`'in tarafı aynı
  anda dönüyor. Kabzanın aynalanması `HoldPoint`'in taraf değişimini büyük ölçüde götürüyor (~13px'e karşı
  ~15px), o yüzden net kayma ~2px. Rahatsız ederse `HandSwapAngle`'ı biraz kaydırmak yerine `HandOffsetRight`
  ile `HandOffsetLeft`'i simetrik yapmak daha temiz (şu an `8` ve `-7`).
- **Gövde ile silah farklı açılarda dönüyor.** Gövde 67.5°'de (`CurrentDir`), silah 90°'de. Aradaki bantta hero
  sola bakarken silah sağ elde duruyor. İstenirse `DirectionUtils`'in yayları da 90'a hizalanabilir, ama o
  gövde animasyonlarının tamamını etkiler.
- **Yerdeki silahın depth'i.** `Depth` yalnızca `CurrentGun` için yazılıyor. Sırt dönükken bırakılan bir silah
  son değerini korur. Holstered silahlar görünmez olduğu için şu an sorun değil.
