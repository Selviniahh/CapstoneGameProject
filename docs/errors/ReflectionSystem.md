# Reflection Sistemindeki Sorunlar ve Doğru Tasarım Yönü

Bu belge mevcut reflection/inspector sisteminin bilinen problemlerini ve ileride
uygulanabilecek doğru tasarım yönünü kaydeder. Buradaki maddeler henüz yapılmış
değişiklikleri değil, mevcut teknik borcu ve önerilen çözümü anlatır.

## Kısa değerlendirme

Mevcut sistem son derece basit veya değersiz değildir. Aşağıdaki teknikleri bir
araya getirmektedir:

- `Boost.Describe` ile class, base class, member ve enum metadata'sı
- `Boost.MP11` ile compile-time metadata dolaşımı
- RTTI, `std::type_index` ve `dynamic_cast` ile runtime type dispatch
- ImGui üzerinde otomatik property inspector üretimi

Bu, bir öğrenme veya küçük oyun projesi için iyi bir prototiptir. Ancak sistem
şu an genel amaçlı veya production seviyesinde bir reflection sistemi değildir.
Daha doğru tanımı, **Boost.Describe kullanan bir debug inspector** sistemidir.

## Mevcut akış

Sistem kabaca şu şekilde çalışır:

```text
BOOST_DESCRIBE_CLASS
        |
        v
Reflection::PopulateReflection<T>()
        |
        v
Boost.MP11 ile base ve member dolaşımı
        |
        v
ShowImGuiWidget<T>()
        |
        v
ImGui property çizimi/düzenlemesi
```

Runtime'da doğru template specialization'a ulaşmak için ayrıca şu akış vardır:

```text
GameClass* / GameObjectBase*
        |
        v
std::type_index(typeid(*object))
        |
        v
TypeRegistry içindeki handler
        |
        v
Reflection::PopulateReflection<GerçekTip>()
```

## 1. Aynı metadata birden fazla yerde tutuluyor

Bir sınıfın kalıtım ve reflection bilgisi şu anda üç ayrı yerde tekrar
edilebiliyor:

1. Gerçek C++ class declaration'ı
2. `BOOST_DESCRIBE_CLASS` içindeki base listesi
3. `REGISTER_BASE_CLASS(Derived, Base)` çağrısı

Bunlardan biri değişip diğerleri değişmediğinde compiler her zaman hata
vermiyor. Sistem yanlış type bilgisiyle çalışmaya devam edebiliyor.

### Mevcut somut uyuşmazlık: AmmoIndicatorsUI

Gerçek declaration ve Boost metadata'sı:

```cpp
class AmmoIndicatorsUI : public GameObjectBase
```

```cpp
BOOST_DESCRIBE_CLASS(AmmoIndicatorsUI, (GameObjectBase), ...)
```

Fakat `TypeRegistry::InitializeTypeRegistry()` içinde:

```cpp
REGISTER_BASE_CLASS(AmmoIndicatorsUI, Hero);
```

Bu kayıt gerçeğe aykırıdır. Custom `TypeID` sistemi bu kayda güvendiği için
`AmmoIndicatorsUI::IsA<Hero>()` yanlış biçimde `true` dönebilir. Ardından
`As<Hero>()` içindeki `static_cast<Hero*>` undefined behavior oluşturabilir.

### Mevcut somut uyuşmazlık: TimerComponent

Gerçek declaration:

```cpp
class TimerComponent : public ComponentBase
```

Boost metadata'sı:

```cpp
BOOST_DESCRIBE_CLASS(TimerComponent, (GameClass), ...)
```

Buradaki base metadata da gerçek C++ kalıtımından farklıdır. Base member'ların
inspector'da eksik görünmesine veya ileride eklenecek otomatik kalıtım
işlemlerinin yanlış çalışmasına neden olabilir.

### Sonuç

Kalıtım metadata'sının tek kaynağı olmalıdır. Bu projede uygun kaynak
`BOOST_DESCRIBE_CLASS` içindeki base listesidir. Ayrı ve manuel
`REGISTER_BASE_CLASS` çağrıları kaldırılmalı veya Boost metadata'sından otomatik
üretilmelidir.

## 2. Type registration tamamen manuel ve kolay unutuluyor

Her reflected concrete type'ın ayrıca
`TypeRegistry::InitializeTypeRegistry()` içine elle eklenmesi gerekiyor:

```cpp
RegisterType<Hero>();
RegisterType<GunBase>();
RegisterType<BulletMan>();
```

Bu liste büyüdükçe yeni bir sınıfın unutulması kaçınılmaz hale gelir. Mevcut
projede metadata'sı bulunan veya factory ile oluşturulan fakat registry'de
bulunmayan örnekler vardır:

- `BaseHealthComp`
- `TimerComponent`
- `ReloadText`
- `AK47AnimComp`
- `MagnumAnimComp`

Bu tiplerden biri doğrudan `TypeRegistry::ProcessObject()` ile işlendiğinde tam
tip eşleşmesi bulunamaz. Mevcut fallback yalnızca `GameClass` metadata'sını
işlediği için property'ler sessizce kaybolabilir.

Sessiz fallback, eksik registration hatasını görünmez hale getirmektedir.
Development build'de en azından belirgin bir log/assert üretmesi gerekir.

## 3. Registry'nin vaat ettiği type kapsamı ile implementation uyuşmuyor

`RegisterType<T>()` yorumlarına göre `Animation` gibi `GameObjectBase` olmayan
utility sınıfları da kaydedilebilmektedir. Fakat handler yalnızca şu koşulda
reflection çalıştırır:

```cpp
if constexpr (std::is_base_of_v<GameObjectBase, T>)
```

Bu nedenle aşağıdaki kayıtlar pratikte kullanışlı bir handler üretmez:

```cpp
RegisterType<GameClass>();
RegisterType<Animation>();
```

`Animation`, `GameClass` tabanlı olmasına rağmen `GameObjectBase` tabanlı
değildir. Registry API'si gerçekten bütün `GameClass` tiplerini destekleyecekse
koşul ve dispatch modeli buna göre tasarlanmalıdır. Aksi halde API yalnızca
`GameObjectBase` kabul ettiğini açıkça belirtmelidir.

## 4. `const_cast` ile property düzenleniyor

`Reflection::PopulateReflection()` şu anda nesneyi const reference olarak alır:

```cpp
static void PopulateReflection(const T& object)
```

Member'ı ImGui üzerinden değiştirebilmek için daha sonra const kaldırılır:

```cpp
member_type& value = const_cast<member_type&>(object.*descriptor.pointer);
```

Çağrılan gerçek nesne const değilse bu tesadüfen güvenli çalışabilir. Gerçekten
const bir nesne gönderildiğinde onu değiştirmek undefined behavior'dır.

Editable inspector açıkça `T&` almalıdır. Read-only inspector gerekiyorsa ayrı
bir `const T&` yolu bulunmalı ve widget'lar disabled/read-only çizilmelidir.

## 5. Birbirini tekrar eden üç type sistemi var

Projede aynı anda şunlar kullanılıyor:

- C++ RTTI: `typeid`, `std::type_index`, `dynamic_cast`
- Custom integer `TypeID`
- Boost.Describe base/member metadata'sı

Bu sistemlerin her biri kendi doğruluk kaynağına sahiptir. Birbirlerinden
koptuklarında yanlış sonuç üretirler.

Custom `TypeID` değerleri process içinde kullanılabilir; fakat registration
sırasına göre üretildikleri için build'ler veya çalıştırmalar arasında kalıcı
değildir. Bu ID'ler save file, network protocol veya serialized asset içine
yazılmamalıdır.

`TypeID::IsBaseOf()` manuel base graph üzerinde recursive dolaşır ve cycle
koruması yoktur. Yanlış registration ile graph'a cycle eklenirse sonsuz
recursion oluşabilir.

En tehlikeli kısım `As<T>()` fonksiyonudur:

```cpp
return IsA<T>() ? static_cast<T*>(this) : nullptr;
```

Buradaki cast'in güvenliği tamamen manuel `TypeID` graph'ının doğru olmasına
bağlıdır. Graph yanlışsa compiler koruma sağlayamaz ve undefined behavior
oluşur.

Küçük proje için mümkün olduğunca gerçek C++ RTTI kullanılmalıdır. Custom type
graph gerçekten gerekli olacaksa kalıtım bilgisi manuel girilmemeli, tek
metadata kaynağından otomatik üretilmelidir.

## 6. Reflection ile ImGui birbirine bağlı

`Reflection::PopulateReflection()` metadata'yı dolaşırken doğrudan ImGui
fonksiyonları çağırır. Bu nedenle aynı metadata kolayca şu alanlarda
kullanılamaz:

- Save/load ve serialization
- Network replication
- Property karşılaştırma
- Undo/redo
- Runtime console
- Unit test
- Başka bir editor UI sistemi

Daha genel tasarımda reflection katmanı property metadata'sı sunmalı, Inspector
katmanı ise bu metadata'yı ImGui ile çizmelidir:

```text
Reflection metadata
        |
        +--> ImGui Inspector
        +--> Serializer
        +--> Debug Console
        +--> Undo/Redo
```

Bu oyun yalnızca debug inspector istiyorsa mevcut coupling kabul edilebilir;
fakat sistem genel engine reflection'ı olarak adlandırılmamalıdır.

## 7. Widget sistemi büyüdükçe zor yönetilir

Her desteklenen tip için `ShowImGuiWidget<T>` specialization'ı yazılması
gerekiyor. Desteklenmeyen bir tip geldiğinde default implementation her frame
`stderr` ve ImGui çıktısı üretebiliyor. Bu hem log spam hem de gereksiz runtime
maliyeti oluşturur.

Ek problemler:

- Property attribute sistemi yoktur (`read-only`, minimum, maximum, step,
  tooltip, category gibi).
- Enum'lar yalnızca yazı olarak gösterilir; genel bir combo editor yoktur.
- Container, optional, variant ve pointer türleri için ortak recursive dispatch
  sınırlıdır.
- Unsupported type kontrolü mümkün olduğunda compile time veya registration
  sırasında yapılmalıdır.

Format string olarak dinamik metin verilmemelidir:

```cpp
ImGui::Text(ErrorMessage.c_str());
ImGui::Text(EnumToString(value));
```

Bunun yerine aşağıdakilerden biri kullanılmalıdır:

```cpp
ImGui::TextUnformatted(ErrorMessage.c_str());
ImGui::Text("%s", EnumToString(value));
```

## 8. Her program çalıştırıldığında kaynak taramak doğru çözüm değildir

Reflection registry oluşturmak için oyun executable'ı açılırken `.h` veya
`.cpp` dosyalarının taranması önerilmez.

Runtime source scanning şu nedenlerle yanlış katmandadır:

- Oyunun kaynak dosyaları release paketinde bulunmayabilir.
- Başlangıç süresini artırır.
- Kaynak kod ile derlenmiş binary'nin aynı olduğunun garantisi yoktur.
- C++ syntax'ını regex ile güvenilir biçimde parse etmek mümkün değildir.
- Template, macro, namespace ve conditional compilation kolayca yanlış okunur.

Bu proje için runtime scanner yazılmamalıdır.

## Önerilen çözüm: explicit self-registration

Bu proje ölçeğinde ayrı bir code generator yerine her reflected concrete type
tek satırla kendisini kaydetmelidir:

```cpp
ETG_REGISTER_REFLECTED_TYPE(Hero)
ETG_REGISTER_REFLECTED_TYPE(GunBase)
ETG_REGISTER_REFLECTED_TYPE(BulletMan)
```

Bu macro kavramsal olarak bir static registrar oluşturup aşağıdaki çağrıyı
yapmalıdır:

```cpp
TypeRegistry::RegisterType<Hero>();
```

Önerilen sorumluluk dağılımı:

```text
Gerçek C++ declaration
    Gerçek kalıtımın kaynağı

BOOST_DESCRIBE_CLASS
    Reflected member ve base metadata'sının tek kaynağı

ETG_REGISTER_REFLECTED_TYPE(T)
    Runtime type -> metadata/handler eşlemesini kuran tek opt-in noktası

Inspector
    Metadata'yı ImGui widget'larına dönüştürür
```

Bu tasarımda ayrıca `REGISTER_BASE_CLASS` yazılmamalıdır. `RegisterType<T>()`,
`boost::describe::describe_bases<T, ...>` listesini compile time dolaşarak base
ilişkilerini gerekiyorsa otomatik kaydetmelidir.

Registry nesnesi function-local static olarak tutulursa global static
initialization order problemleri azaltılabilir:

```cpp
static RegistryMap& Registry()
{
    static RegistryMap registry;
    return registry;
}
```

Static registrar yaklaşımında linker dead stripping ve static library
davranışları ayrıca düşünülmelidir. Proje shared library kullandığı için bu
risk daha düşüktür; yine de registration'ın test edilmesi gerekir.

## Basit ve güvenli alternatif: merkezi compile-time type listesi

Self-registration istenmiyorsa tek bir type listesi kullanılabilir:

```cpp
using ReflectedTypes = boost::mp11::mp_list<
    Hero,
    GunBase,
    AK47,
    Magnum,
    BulletMan
>;
```

Liste tek bir compile-time döngüyle kaydedilebilir:

```cpp
boost::mp11::mp_for_each<ReflectedTypes>([](auto wrappedType)
{
    using T = typename decltype(wrappedType)::type;
    TypeRegistry::RegisterType<T>();
});
```

Bu çözüm tam otomatik değildir; fakat kalıtım bilgisini tekrar etmez, kolay
debug edilir ve küçük proje için oldukça güvenlidir.

## Code generation ne zaman doğru olur?

Unreal benzeri geniş bir engine hedefleniyorsa ayrı bir reflection tool mantıklı
olabilir. Bu tool oyun çalışırken değil, **build sırasında** çalışmalıdır.

Doğru akış:

```text
İşaretlenmiş header değişti
        |
        v
Reflection tool header'ı parse etti
        |
        v
TypeRegistry.gen.cpp / TypeName.gen.hpp üretildi
        |
        v
Normal C++ compiler generated dosyaları derledi
        |
        v
Executable çalıştırıldı
```

Sağlam bir generator şu özelliklere sahip olmalıdır:

- Yalnızca açıkça işaretlenmiş tipleri işler (`UCLASS`/`UPROPERTY` benzeri).
- Regex yerine Clang AST/LibTooling gibi gerçek bir C++ parser kullanır.
- `compile_commands.json` üzerinden gerçek compiler flag'lerini bilir.
- Generated dosyaları source tree yerine build directory'ye yazar.
- Header dependency'lerini CMake/Ninja'ya bildirir.
- Yalnızca input değiştiğinde yeniden çalışır.
- Generated dosya içeriği değişmediyse timestamp'i gereksiz yere güncellemez.

Her build'de bütün kaynakları körlemesine tarayan veya her executable run'ında
registry yazan bir sistem doğru değildir.

Bu proje için Clang tabanlı generator yazmak, çözülmek istenen problemden daha
büyük bir compiler-tooling projesi oluşturur. Mevcut ölçekte self-registration
veya merkezi type listesi daha doğru seçimdir.

## Önerilen refactor sırası

1. Yanlış base kayıtlarını hemen düzelt.
2. Metadata'sı olup registry'de olmayan concrete tipleri tespit eden test ekle.
3. `PopulateReflection(const T&)` ve `const_cast` kullanımını `T&` olarak
   değiştir.
4. `REGISTER_BASE_CLASS` tekrarını kaldır; base bilgisini Boost.Describe'dan
   üret.
5. `InitializeTypeRegistry()` listesini self-registration veya tek type listesi
   ile değiştir.
6. Custom `TypeID` gerçekten gerekli mi ölç; gereksizse RTTI lehine kaldır.
7. Reflection metadata dolaşımı ile ImGui çizimini birbirinden ayır.
8. Read-only/range/category gibi property attribute'larını yalnız ihtiyaç
   oluştuğunda ekle.
9. Code generation'ı ancak proje gerçek bir engine/tooling platformuna
   dönüşürse değerlendir.

## Hedef durum

Başarılı refactor sonunda yeni bir reflected class eklemek için en fazla şu iki
opt-in noktası yeterli olmalıdır:

```cpp
BOOST_DESCRIBE_CLASS(NewEnemy, (EnemyBase), (Health, Speed), (), ())
ETG_REGISTER_REFLECTED_TYPE(NewEnemy)
```

Şunlar ayrıca yazılmamalıdır:

- Manuel base relationship
- Merkezi fonksiyon içinde ikinci bir `RegisterType<NewEnemy>()`
- Elle üretilmiş integer type ID eşlemesi
- Runtime source scan

Bu hedef, mevcut sistemin iyi taraflarını korurken metadata drift ve unsafe cast
risklerini önemli ölçüde azaltır.
