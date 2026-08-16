#include "Block.h"
#include "../../Engine/Core/Factory.h"
#include "../../Engine/Core/Components/CollisionComponent.h"
#include "../../Engine/Managers/RenderContext.h"
#include "../../Utils/TextureUtils.h"

namespace ETG
{
    ETG::Block::Block(const ETG::Vector2f& position, const TileType type) : Type(type)
    {
        Position = position;

        //Bir resim dosyasi yerine, 1x1 beyaz pikselin gerilmis hali olarak ciziliyor. Boylece gecici bir tile'in
        //hicbir sanata ihtiyaci olmuyor ve odadaki butun tile'lar tek bir texture'i paylasiyor - ki bu, odanin
        //boyutu ne olursa olsun tek batch demek. Origin o pikselin ortasina dusuyor, dolayisiyla Position hucrenin
        //bir kosesi degil MERKEZI: collision kutusu da Position'a ortalaniyor (bkz. UseManualBounds), ve ikisinin
        //ayni yeri gostermesi, bir grid index'inin tek carpmayla pozisyona cevrilebilmesini saglayan sey
        Texture = GetPixelTexture();
        Origin = {0.5f, 0.5f};
        Scale = {TileSize, TileSize};
        Color = TileDebugColor(Type);

        //Uzerinde yuruyen her seyin arkasinda. SpriteBatch buyuk depth'leri ONCE cizer, hero da -1'de duruyor
        Depth = 10.f;

        CollisionComp = ETG::CreateGameObjectAttached<CollisionComponent>(this);

        //Sanat, olceklenmis tek bir piksel; yani ondan alinacak bounds hucrenin ortasinda 1x1'lik bir kutu olurdu.
        //UseManualBounds'un var olma sebebi tam olarak bu durum - buraya yazilan sayi gercegin kendisi, sprite ise
        //onun yerine gecen sey
        CollisionComp->UseManualBounds = true;
        CollisionComp->ManualBoundsSize = {TileSize, TileSize};
        CollisionComp->CollisionRadius = 0.f; //bir tile tam olarak kendi hucresidir; radius komsulariyla cakisirdi

        //Mover'larin BlockingMask'larinda isimlendirdikleri sey Obstacle. Mask'in bos olmasinin sebebi ise duvarin
        //hicbir seyi izlememesi: ona hicbir zaman bir sey haber verilmez, o OKUNUR - hem de icine girmek uzere olan
        //tarafindan
        CollisionComp->Layer = CollisionLayer::Obstacle;
        CollisionComp->Mask = CollisionLayer::None;
        CollisionComp->Name = "Block";

        //Sadece kati kategoriler isin icine giriyor. Zemin tile'i collider'siz kalmak yerine (kapali) collider'ini
        //tutuyor; boylece bir hucreyi calisma aninda zeminden duvara cevirmek tek bir flag, bastan kurmak degil
        CollisionComp->SetCollisionEnabled(IsTileSolid(Type));

        //Acikca cagriliyor; ilk frame'in sonundaki collision pasinin yapmasina birakilmiyor: hareket sorgusu bu
        //bounds'u frame'in ortasindan okuyor, ve ilk frame'de bu, pas hic calismadan ONCEKI an demek. Bu satir
        //olmazsa duvarin yanina spawn olan bir hero, o duvarin icinden tam bir kez gecer
        CollisionComp->Initialize();

        //Yukarida set edilen texture, origin, scale ve rengi draw properties'e yayinliyor. Bilerek bu, ve
        //Initialize() DEGIL: Initialize origin'i Texture->getSize() / 2 diye yeniden hesapliyor, ve 1x1 bir
        //texture'in boyutunun unsigned aritmetikte yarisi 0 eder - bu da hucreyi sessizce sol ust kosesinden
        //sabitler ve odadaki her tile, carpistigi yerin yarim hucre sag altina cizilirdi
        ComputeDrawProperties();
    }

    ETG::Block::~Block() = default;

    void ETG::Block::SetTileType(const TileType type)
    {
        Type = type;
        Color = TileDebugColor(Type);

        if (CollisionComp) CollisionComp->SetCollisionEnabled(IsTileSolid(Type));
    }

    void ETG::Block::Draw()
    {
        if (!IsVisible) return;

        GameObjectBase::Draw();

        //Collider'i olan diger her nesnenin yaptigi cagrinin aynisi; boylece editordeki ShowCollisionBounds
        //anahtari duvarda da hero'daki ile ayni anlama geliyor
        if (CollisionComp) CollisionComp->Visualize(*ETG::RenderContext::Window);
    }
}
