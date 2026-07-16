# Gereksiz Kütüphane Ayrımı ve Sahte Modülerlik

Bu belge, `src` altındaki kodun gerçek dependency sınırları olmadan birden fazla
CMake target'ına ayrılmasıyla oluşan tasarım problemini kaydeder. Mevcut ayrım
kodun daha profesyonel görünmesi amacıyla yapılmıştır; fakat target sayısının
artması tek başına modülerlik sağlamaz.

## Mevcut yapı

Proje kaynakları şu target'lara ayrılmıştır:

```text
etgcore
utils
modifiers
ETG
```

`ETG` executable'ı bunların hepsini ayrı ayrı linklemektedir:

```cmake
target_link_libraries(ETG PRIVATE
        etgcore
        utils
        modifiers
)
```

Ancak bu target'lar gerçekte birbirlerinden bağımsız değildir.

## Temel problem: Target sınırları dependency sınırlarıyla uyuşmuyor

`etgcore` içindeki birçok kaynak `DirectionUtils`, `Math` ve modifier
header'larını doğrudan kullanmaktadır. Buna karşılık `DirectionUtils.cpp`,
`Hero` gibi `etgcore` içinde bulunan oyun sınıflarına bağımlıdır.

Ortaya çıkan ilişki kabaca şöyledir:

```text
etgcore ---> utils
   ^          |
   |----------|
```

Yani `utils`, isminden beklenen bağımsız ve genel amaçlı bir yardımcı kütüphane
değildir. Oyun tiplerini bildiği için `etgcore`'un bir parçasıdır. `modifiers`
da doğrudan silah ve oyun kodu tarafından kullanılan header-only bir proje
bileşenidir; bağımsız bir paket sınırı oluşturmamaktadır.

Executable'ın hem `etgcore` hem `utils` linklemesi bu yanlış dependency
modelini gizleyebilir. Özellikle shared library ve Windows DLL yapısında
karşılıklı dependency; symbol çözümleme, link sırası, export/import ve runtime
yükleme problemlerine dönüşebilir.

## Dosya grupları ile target'lar aynı şey değildir

Kaynakları okunabilirlik için listelere ayırmak normaldir:

```cmake
set(CORE_SOURCES ...)
set(UTILS_SOURCES ...)
set(MODIFIERS_SOURCES ...)
```

Bu listeler yalnızca CMake değişkenidir. Her liste için ayrıca `add_library()`
çağırmak gerekmez. Hepsi tek gerçek target'a verilebilir:

```cmake
add_library(etgcore
        ${CORE_SOURCES}
        ${UTILS_SOURCES}
        ${MODIFIERS_SOURCES}
)
```

Böylece klasör ve kaynak grupları düzenli kalırken gereksiz binary sınırları
oluşturulmaz.

## Bu proje için doğru yön

Mevcut dependency yapısı değişmeden kalacaksa `src` altındaki oyun ve engine
kodunun tek bir `etgcore` kütüphanesinde toplanması daha doğrudur. Executable
yalnızca giriş noktası olan `main.cpp` dosyasını içermeli ve `etgcore`'u
linklemelidir:

```cmake
add_executable(ETG main.cpp)
target_link_libraries(ETG PRIVATE etgcore)
```

Bu yapının avantajları:

- Gerçek kod bağımlılıklarını doğru temsil eder.
- CMake ve linker yapılandırmasını sadeleştirir.
- Shared library sınırındaki karşılıklı dependency riskini kaldırır.
- Ortak compile option, definition ve dependency'lerin tek yerde tutulmasını
  sağlar.
- `etgcore`, ileride test executable'ları tarafından da kullanılabilir.

## Ne zaman ayrı kütüphane yapılmalı?

Bir klasör yalnızca proje ağacında ayrı göründüğü için kütüphane yapılmamalıdır.
Ayrı target için aşağıdakilerden en az biri gibi gerçek bir gerekçe bulunmalıdır:

- Başka projelerde bağımsız olarak yeniden kullanılabilmesi
- Açık ve tek yönlü dependency sınırına sahip olması
- Farklı compiler veya linker ayarlarına ihtiyaç duyması
- Plugin/DLL olarak bağımsız yüklenmesi
- Ayrı kurulması, paketlenmesi veya sürümlenmesi

Örneğin yalnızca temel sayısal tipleri kullanan ve hiçbir oyun sınıfını
bilmeyen genel bir matematik kütüphanesi ayrılabilir. Fakat `Hero` gibi oyun
tiplerine bağımlı bir `DirectionUtils`, bağımsız utility kütüphanesi değildir.

## Sonuç

Fazla target kullanmak profesyonellik göstergesi değildir. İyi modülerlik;
az veya çok target kullanılmasından değil, her target'ın açık bir sorumluluğa
ve tek yönlü dependency ilişkilerine sahip olmasından gelir. Mevcut projede tek
bir `etgcore` target'ı, ayrı `utils` ve `modifiers` target'larından daha dürüst
ve sürdürülebilir bir yapıdır.
