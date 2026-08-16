# Constructor / Initialize / BindEvents — ne nereye gider

Bu projede her sınıfın constructor'ı kendi `Initialize()`'ını çağırıyor. Bir sınıftan türetilince zincir şuna dönüşüyor:

```
BulletMan::BulletMan()
  └─ EnemyBase::EnemyBase()        ← base ctor önce koşar
       └─ EnemyBase::Initialize()  ← 1. kez
  └─ BulletMan::Initialize()
       └─ EnemyBase::Initialize()  ← 2. kez
```

Yani **`Initialize()` sınıf başına bir kez değil, türeme derinliği kadar kez çalışır.** `EventDelegate::AddListener` aynı lambda'yı ikinci kez sorgusuz kabul ettiği için, `Initialize` içinde bağlanan her listener iki kopya olur. Gerçekten olan: bir mermi `ApplyDamage`'i iki kez çağırdı, bir ölüm `OnDeath`'i iki kez broadcast etti, her silahın reload slider'ı aynı silahı iki kez dinledi.

---

## Kural tablosu

| Nereye | Ne gider | Kaç kez çalışır | Örnek |
|---|---|---|---|
| **Constructor** | Sub-object oluşturma (`CreateGameObjectAttached`), ctor parametrelerinin field'lara yazılması, sabit kurulum (`Layer`, `Mask`, `Depth`) | Nesne başına **tam 1** | `CollisionComp = CreateGameObjectAttached<...>(this)` |
| **`BindEvents()`** | **Her `AddListener` çağrısı, istisnasız** | Nesne başına **tam 1** (yalnız kendi ctor'undan) | `HealthComp->OnDeath.AddListener(...)` |
| **`Initialize()`** | Yeniden çalıştırılması **zararsız** olan ayar: sayısal tuning, texture yükleme, sıfırlama | **Belirsiz** — türeme derinliği kadar, editörden de tetiklenebilir | `MovementSpeed = 40.f;` · `CurrentHealth = MaxHealth;` |
| **`Update()`** | Her frame değişen şeyler | Frame başına 1 | pozisyon, timer, animation |

`BindEvents()` `GameObjectBase`'de boş virtual olarak duruyor; her sınıf kendi override'ını yazar.

---

## `BindEvents` yazarken 3 kural

1. **Sınıf yalnızca KENDİ listener'larını bağlar.** Başka sınıfın event'ini bağlamak o sınıfın işi.
2. **Kendi ctor'undan, kendi qualify edilmiş versiyonunu çağır** — `Hero::Hero()` içinden `Hero::BindEvents();`, ctor'un **son satırı** olarak. Ctor nesne başına tam bir kez koşar; işin tamamı buna dayanıyor.
3. **Override asla `Base::BindEvents()` çağırmaz.** Base'in ctor'u kendi `BindEvents`'ini zaten çalıştırdı; zincirlemek onu ikinci kez bağlar.

Ayrıca: **`BindEvents` base pointer üzerinden çağrılmaz.** Virtual dispatch yalnızca en türemiş override'a iner ve aradaki bütün base bağlamalarını sessizce atlar.

```cpp
// Hero.h
protected:
    void BindEvents() override;

// Hero.cpp
ETG::Hero::Hero(...)
{
    ... sub-object'ler ...
    Hero::Initialize();
    Hero::BindEvents();   // son satır
}

void ETG::Hero::BindEvents()
{
    CollisionComp->OnCollisionEnter.AddListener(...);
    HealthComp->OnDamageTaken.AddListener(...);
}
```

---

## Tek istisna: çalışma anında yeniden bağlama

Bazı bağlamalar construction'a değil, bir olaya bağlıdır — hero silah değiştirdiğinde `ReloadSlider::LinkToGun` / `ReloadText::LinkToGun` yeni silahın event'ine abone olur. Bunlar `BindEvents`'e ait değildir, çünkü aynı obje için tekrar tekrar çağrılırlar.

Kural: **böyle bir fonksiyon önce `Clear()`, sonra `AddListener()` yapar.** Aksi hâlde aynı silaha ikinci kez dönüldüğünde ikinci bir canlı listener kalır. `Clear()` ancak o event'i tek bir dinleyicinin dinlediği durumda güvenlidir; birden fazla varsa `AddListener`'ın döndürdüğü handle saklanıp `RemoveListener` ile kaldırılmalıdır.

---

## Yeni sınıf eklerken kontrol listesi

- [ ] `AddListener` yazdığım tek yer `BindEvents()` mi?
- [ ] `BindEvents()`'i yalnızca kendi ctor'umun son satırından, qualify ederek mi çağırdım?
- [ ] Override'ımda `Base::BindEvents()` çağrısı **yok** değil mi?
- [ ] `Initialize()`'a koyduğum her satır ikinci kez çalışsa da doğru sonuç veriyor mu? (`Origin += Offset` gibi biriken satırlar buraya konmaz)
- [ ] Sub-object'leri (`CreateGameObjectAttached`) ctor'da mı oluşturdum? `Initialize`'da oluşturulan bir sub-object ikinci çağrıda yenisiyle değişir ve üzerindeki listener'lar sessizce ölür.

---

## Bugün bu kurala uyan sınıflar

`GameObjectBase` (boş hook) · `Hero` · `EnemyBase` · `ProjectileBase` · `GunBase` · `Magnum` · `SawedOff` · `AK47` · `BaseHealthComp` · `PlatinumBullets` · `DoubleShoot` · `TakeNoDamage`
