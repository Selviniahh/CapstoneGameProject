# Test/ — Unit testler ve Interactive Gameplay testleri

Bu klasörde **iki ayrı test türü** var. İkisi de oyundan bağımsız birer çalıştırılabilir üretir:

| Hedef | Ne yapar | Pencere açar mı |
|---|---|---|
| `ETGUnitTests` | GoogleTest. Saf mantık: açı → yön dönüşümü, `StatModifier` formülü, `Math` yardımcıları | Hayır, konsol |
| `ETGInteractiveTests` | Gerçek oyun motorunu **boş bir dünyayla** açar. Yüklü test kendi dünyasını kurar; sen oynarsın, testler `PENDING → PASSED` olur | Evet |

---

## 1. Hızlı başlangıç

```bash
# Her şeyi derle (oyun + iki test hedefi de). Hiçbir gate açık değil.
cmake -S . -B build -G Ninja
cmake --build build

# Unit testler
./build/bin/ETGUnitTests
ctest --test-dir build            # aynı şey, CTest üzerinden

# Interactive gameplay testleri
./build/bin/ETGInteractiveTests
```

> `deps/googletest` submodule'ü çekili değilse unit testler **sessizce** devre dışı kalır (configure hata vermez).
> Çekmek için: `git submodule update --init deps/googletest`

---

## 2. "Runtime'dan önce çalıştırma" (gate'ler)

İstenen davranış: *bazen ikisi de oyundan önce çalışsın, bazen sadece biri, bazen hiçbiri.*
Bunu iki CMake seçeneği belirler. Gate açıkken **ETG hedefi o testlere bağımlı hale gelir**: oyun derlenmeden
önce testler çalışır, testler kırmızıysa build durur ve oyun hiç başlamaz.

| Ne istiyorsun | Configure komutu |
|---|---|
| Sadece oyun (varsayılan) | `cmake -S . -B build` |
| Önce unit testler, sonra oyun | `cmake -S . -B build -DETG_RUN_UNIT_TESTS_BEFORE_GAME=ON` |
| Önce interactive testler, sonra oyun | `cmake -S . -B build -DETG_RUN_INTERACTIVE_TESTS_BEFORE_GAME=ON` |
| Önce ikisi de, sonra oyun | `cmake -S . -B build -DETG_RUN_UNIT_TESTS_BEFORE_GAME=ON -DETG_RUN_INTERACTIVE_TESTS_BEFORE_GAME=ON` |
| Test hedeflerini hiç derleme | `cmake -S . -B build -DETG_BUILD_UNIT_TESTS=OFF -DETG_BUILD_INTERACTIVE_TESTS=OFF` |

Sonra her zamanki gibi `cmake --build build --target ETG` (ya da CLion'da Run ETG) — gate'ler o sırada çalışır.
Interactive gate'te pencere açılır; **pencereyi kapattığında** build devam eder ve oyun başlar.

### Tüm seçenekler

| Seçenek | Varsayılan | Anlamı |
|---|---|---|
| `ETG_BUILD_UNIT_TESTS` | `ON` | `ETGUnitTests` hedefi derlensin mi |
| `ETG_BUILD_INTERACTIVE_TESTS` | `ON` | `ETGInteractiveTests` hedefi derlensin mi |
| `ETG_RUN_UNIT_TESTS_BEFORE_GAME` | `OFF` | Oyun build'inden önce unit testleri çalıştır |
| `ETG_RUN_INTERACTIVE_TESTS_BEFORE_GAME` | `OFF` | Oyun build'inden önce interactive oturumu aç |
| `ETG_UNIT_TESTS_FAIL_BUILD` | `ON` | Kırmızı unit test build'i durdursun mu |
| `ETG_INTERACTIVE_TESTS_FAIL_BUILD` | `ON` | `FAILED` bir gameplay check'i build'i durdursun mu |
| `ETG_INTERACTIVE_TESTS_STRICT` | `OFF` | Gate ayrıca **her testin** bir sonuca bağlanmış olmasını şart koşsun mu (hiç açılmamış / hâlâ `PENDING` test = başarısız) |

`ETGInteractiveTests` çıkış kodları: `0` sorun yok · `1` en az bir check `FAILED` (ya da bir testin `SetUp`'ı patladı) ·
`2` (`--strict`) testler tamamlanmadı.

**Verdict dosyası.** Gate açıkken host `--verdict-file=<build>/Test/interactive-tests-verdict.txt` ile çalışır ve
sonucu (`OK` / `FAILED` / `INCOMPLETE`) daha kapanmadan diske yazar. Gate bu dosyaya **sadece** process sıfırdan
farklı bir kodla bittiğinde bakar: motorun kapanış aşamasında çökmesi (driver teardown vb.) ile gerçekten kırmızı
bir gameplay check'i birbirinden ayırabilmek için. Testler geçmişse çökme uyarı olarak geçilir, build durmaz.

---

## 3. Yeni bir interactive test yazmak

Üç adım. Tek dosya açıyorsun, ortasında hiçbir merkezi listeyi düzenlemiyorsun.

### Adım 1 — dosyayı yaz: `Test/Interactive/Tests/MyThingTest.cpp`

```cpp
#include "../Framework/InteractiveTest.h"
#include "../Framework/InteractiveTestRegistry.h"
#include "../Framework/TestEnvironment.h"
#include "Engine/Core/Components/BaseHealthComp.h"
#include "Game/Characters/Hero/Hero.h"
#include "Game/Characters/Hero/Components/HeroMoveComp.h"

using namespace ETG;
using namespace ETG::Testing;

namespace
{
    class MyThingTest final : public InteractiveTest
    {
    public:
        //Panelin üstünde görünen yönerge (opsiyonel)
        std::string GetInstructions() const override { return "WASD ile sağa doğru 200 px yürü."; }

        //1) BU TESTİN DÜNYASI. Boş bir dünya gelir, sen doldurursun.
        void SetUp(TestEnvironment& env) override
        {
            Hero* hero = env.SpawnHero({0.f, 0.f});

            //Karakteri BU TEST İÇİN değiştir - oyunun level'ına dokunmadan
            hero->GetMoveComp()->MaxSpeed = 400.f;
            hero->HealthComp->CurrentHealth = 999.f;

            //2) İddialar. Panelde PENDING olarak görünürler
            WalkedRight = AddCheck("Hero 200 px sağa gidebiliyor mu?", "D tuşuna basılı tut");
        }

        //3) Her frame çağrılır. Check'leri burada sonuçlandır
        void Update(TestEnvironment& env) override
        {
            const Hero* hero = env.GetHero();
            if (!hero) return;

            WalkedRight.Progress(std::to_string((int)hero->GetPosition().x) + " / 200 px");
            WalkedRight.PassIf(hero->GetPosition().x >= 200.f);
        }

    private:
        CheckHandle WalkedRight;
    };
}

//4) Panelde görünmesi için tek satır: sınıf, kategori, görünen ad
ETG_INTERACTIVE_TEST(MyThingTest, "Hero", "Hero sağa yürüyebiliyor")
```

### Adım 2 — `Test/CMakeLists.txt` içinde `ETG_INTERACTIVE_TEST_SOURCES` listesine ekle

```cmake
set(ETG_INTERACTIVE_TEST_SOURCES
        Interactive/Tests/HeroDirectionTest.cpp
        Interactive/Tests/GunShotSpeedTest.cpp
        Interactive/Tests/EnemyEngagementTest.cpp
        Interactive/Tests/MyThingTest.cpp        # <-- yeni
)
```

### Adım 3 — çalıştır

`./build/bin/ETGInteractiveTests` → sol paneldeki **All tests** listesinden testini seç.

---

## 4. API özeti

Ayrıntılı açıklamalar başlık dosyalarının içinde; burada sadece ne nerede:

### `TestEnvironment` — dünyayı kur ve oku (`Framework/TestEnvironment.h`)

```cpp
// Spawn
Hero*  hero = env.SpawnHero({0.f, 0.f});          // ÖNCE bu: düşmanlar ctor'da Hero::Get() yakalar
auto*  enemy = env.SpawnEnemy<BulletMan>({140.f, 0.f});
auto*  ak   = env.GiveHeroGun<AK47>();            // spawn + hero'nun eline ver
auto*  item = env.Spawn<PlatinumBullets>();       // genel hâli: her world object

// Sorgula
GunBase* gun = env.GetHeroGun();
std::vector<ProjectileBase*> bullets = env.FindAll<ProjectileBase>();
ProjectileBase* first = env.FindFirst<ProjectileBase>();
size_t enemyCount = env.CountOf<BulletMan>();

// Zaman
float dt = TestEnvironment::DeltaSeconds();
float t  = env.SecondsSinceSetUp();
```

Spawn edilen her şey test bittiğinde/yeniden başlatıldığında otomatik yok edilir.

### `CheckHandle` — iddiayı sonuçlandır (`Framework/InteractiveTest.h`)

```cpp
Check.Pass("detay");                 Check.Fail("neden");
Check.PassIf(kosul);                 Check.FailIf(kosul, "neden");
Check.Progress("3 / 8");             // PENDING iken canlı metin
Check.ExpectNear(olculen, beklenen, tolerans, " px");   // ölçüm testleri için tek satır
Check.IsPassed(); Check.IsPending(); Check.Reset();
```

İlk verdict kalıcıdır: bir check `PASSED`/`FAILED` olduktan sonra her frame çağrılan `PassIf` onu değiştirmez.

### `Framework/TestWatchers.h` — tekrar eden defter tutma işleri

```cpp
DirectionCoverage Coverage;   // 8 yönün hangileri görüldü → Observe / IsComplete / Describe
TravelProbe Probe;            // bir nesne ne kadar yol gitti, ne kadar sürede → Start / Tick / GetDisplacement
Stopwatch Watch;              // oyunun kendi saatiyle saniye sayacı (pencere odağı gidince durur)
```

İkinci bir testte aynı defter tutmayı yazdığını fark edersen, yeri burasıdır.

---

## 5. Nasıl çalışıyor (kısaca)

* `ETGInteractiveTests` oyunun `GameManager`'ını **aynen** kullanır. Tek fark:
  `GameManager::LevelSpawnOverride` set edilir, böylece `SpawnInitialLevel` yerine sadece
  `InteractiveTestRunner` spawn edilir. Dünyadaki her şeyi testler koyar.
* Aynı anda **tek test** yüklüdür: her test tüm dünyanın sahibidir (kendi Hero'su, kendi düşmanları),
  iki test aynı anda çalışsa tek `Hero::Get()` üstünde kavga ederlerdi.
* Test değiştirmek iki frame sürer — eskiler `MarkForDestroy` ile işaretlenir, `GameManager` frame sonunda
  süpürür, **sonraki** frame yeni testin `SetUp`'ı çağrılır. Aynı frame'de yapılsaydı yeni Hero eski Hero
  hâlâ yaşarken kurulur, ctor'da `Hero::Get()` yakalayan her düşman ölü pointer tutardı.
* Test'ten fırlayan exception oturumu düşürmez: panelde kırmızı satır olarak gösterilir.

## 6. Yeni bir unit test yazmak

`Test/Unit/` altına dosya ekle, `ETG_UNIT_TEST_SOURCES` listesine yaz, bitti:

```cpp
#include <gtest/gtest.h>
#include "Engine/Core/Stats/StatModifier.h"

TEST(StatModifier, RemovalIsExact) { /* ... */ }
```

Kural: **fonksiyona sayı verip cevabı kontrol edebiliyorsan** unit test; cevabı görmek için birinin
oynaması gerekiyorsa interactive test.
