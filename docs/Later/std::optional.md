std::optional<T>, içinde ya bir T değeri bulunan ya da hiçbir değer bulunmayan bir kutudur.

std::optional<StateEnum> LeafId{};

Şu iki durumdan birini temsil eder:

LeafId boş                 → Bu node composite node
LeafId içinde StateEnum var → Bu node gerçek bir leaf state

Örneğin basit kullanım:

std::optional<int> number;

Başlangıçta boştur:

number.has_value() == false

İçine değer koyabiliriz:

number = 42;

number.has_value() == true

Değeri okuyabiliriz:

int value = *number;

veya:

int value = number.value();

Tekrar boşaltabiliriz:

number = std::nullopt;

### StateNode içinde neden kullanılıyor?

İki constructor var:

explicit StateNode(std::string name)
: Name(std::move(name))
{
}

Bu constructor composite node oluşturuyor. LeafId verilmediği için boş kalıyor:

auto alive = CreateNode("Alive");

// alive->LeafId boş

Diğer constructor:

StateNode(std::string name, StateEnum leafId)
: Name(std::move(name)), LeafId(leafId)
{
}

Bu bir leaf oluşturuyor:

auto idle = CreateLeaf("Idle", HeroStateEnum::Idle);

// idle->LeafId içinde HeroStateEnum::Idle var

Yani:

Alive
LeafId = boş

Idle
LeafId = HeroStateEnum::Idle

Değer kontrolü projede şöyle yapılıyor:

if (!leaf->LeafId.has_value())
{
throw std::runtime_error("Leaf enum değeri bulunamadı");
}

Sonra içindeki değer alınıyor:

return *leaf->LeafId;

Neden doğrudan StateEnum LeafId{} kullanılmamış? Çünkü enum’un 0 değeri gerçek bir state olabilir:

enum class HeroStateEnum
{
Idle, // 0
Run,
Dash
};

Bu durumda 0, “değer yok” anlamına gelemez çünkü Idle zaten 0dır. std::optional bu ayrımı temiz şekilde yapar:

std::nullopt         → Hiçbir state yok
HeroStateEnum::Idle  → Gerçekten Idle state’i var