#pragma once
#include <cstdint>
#include "../ComponentBase.h"
#include "../SafeRegistry.h"
#include "../Events/EventDelegate.h"

namespace ETG
{
    class GameObjectBase;
    class CollisionEventData;
    class CollisionSystem;

 // - CollisionComponent: Her objenin collision ile ilgili verisini taşır.
 // - CollisionSystem: Bütün collider’ları merkezi olarak kontrol eder ve sonucu üretir.
//Yani component “Benim kutum, layer’ım ve maskem ne?” der; system ise “Kim kiminle çarpıştı ve hangi event çalışmalı?” sorusunu çözer.
    
  // Eskiden her obje kendi Update() fonksiyonunda collision kontrolü yapsaydı şu problem oluşurdu:
  //
  // 1. Hero hareket eder ve Enemy’nin eski pozisyonuna bakar.
  // 2. Enemy daha sonra hareket eder ve Hero’nun yeni pozisyonuna bakar.
  // 3. Aynı frame’de Hero “çarpışmadık”, Enemy “çarpıştık” diyebilir.
  //
  // Sonuç, objelerin update/spawn sırasına bağlı olurdu.
  //
  // GameManager'da tüm objeler'in Update'i çalışıyor ardından collision sistemi yalnızca bir kere çalışıyor:
  //
  // for (const auto& obj : WorldObjects)
  //     obj->Update();
  //
  //   CollisionSystem::Update();
  //
  //   Böylece bütün collision testleri aynı andaki pozisyonları kullanıyor
  // 
    
    
    
    //CollisionComp->Mask = CollisionLayer::Enemy | CollisionLayer::Projectile; 
    //dediğim zaman hem enemy hem projectile ile benim collisinım tepkiye girsin gerisi de girmesin diyoruz sağlamasını yapalım 
    
 //    Hero’nun ayarları:
 //
 // CollisionComp->Layer = CollisionLayer::Hero;
 //
 //    CollisionComp->Mask =
 //        CollisionLayer::Enemy |
 //        CollisionLayer::Projectile;
 //    
 //    Enemy       0010
 //  Projectile  0100
 //               ---- OR
 //  Mask        0110 = 6
 //
 //  ## Hero, Enemy’yi kontrol ederken
 //
 //  Enemy’nin layer değeri:
 //
 //  otherComp->Layer = Enemy = 0010
 //
 //  Mask kontrolü:
 //
 //  Mask         0110
 //  Enemy Layer  0010
 //                ---- AND
 //  Sonuç        0010 = 2
 //
 //  Yani:
 //
 //  Mask & otherComp->Layer
 //
 //  sonucu 2 olur. Sıfır olmadığı için boolean olarak true kabul edilir.
 //    Sonucun 0 olmasi su anlama gelir: 
 //    > “Diğer objenin layer’ı benim maskemde yok; bu objeyi collision kontrolüne dahil etme.”
    namespace CollisionLayer
    {
        enum : uint32_t
        {
            None = 0, //0
            Hero = 1u << 0, //1
            Enemy = 1u << 1, //2
            Projectile = 1u << 2, //4
            Pickup = 1u << 3, //Guns and items lying in the room, waiting for someone to walk over them           //8

            //Odanin kendisi: duvarlar ve levelin insa edildigi her sey. Kimsenin Mask'ina konulmamasi BILEREK
            //boyle - duvar, olup bittikten sonra tepki verilecek bir event degil; zaten hicbir zaman icine
            //giremedigin bir yer. Onun yerine BlockingMask'ta isimlendiriliyor, onu da hareket pasi okuyor
            Obstacle = 1u << 4, //16

            Default = 1u << 31, //Whatever never named a layer. Paired with Mask = All it behaves as before       //2147483648
            All = ~0u, //4294967295u  //tüm bitler 1111111111111
        };
    }

    class CollisionComponent : public ComponentBase
    {
    public:
        CollisionComponent();
        ~CollisionComponent() override;

        void Initialize() override;

        // Objenin ne olduğunu söyler.
        uint32_t Layer = CollisionLayer::Default;
        
        // Objenin hangi tür objeleri dinlediğini söyler.
        uint32_t Mask = CollisionLayer::All;

        //Bu obje icin hangi layerlar KATI - yani en bastan icine girmesine izin verilmeyenler.
        //Mask'tan farkli bir soru soruyor, fark da sorunun NE ZAMAN cevaplandiginda. Mask gecmis zaman: frame
        //bitti, cakistik, al sana event. BlockingMask ise gelecek zaman: mover bu frame `delta` kadar yol
        //gidecek, CollisionSystem::MoveAndSlide o yolu kisaltiyor ve cakisma hic yasanmiyor. Bu yuzden duvar
        //hicbir event atmaz - rapor edilecek bir sey yok, cunku kimse icine girmedi.
        //
        //Varsayilan olarak bos. Yani hicbir sey yazmayan bir obje, bu ozellik hic yokmus gibi aynen eskisi gibi
        //hareket etmeye devam eder. Buraya bir layer yazmak isin sadece yarisi: mover'in BaseMoveComp'una da
        //dunyanin durduracagi kutunun collider'i verilmeli (BaseMoveComp::BodyCollider)
        uint32_t BlockingMask = CollisionLayer::None;

        //Radius to expand collision box beyond the texture boundaries
        float CollisionRadius = 0.0f;

        //Bounds normally come off whatever the owner is drawing - its current animation frame, or its texture.
        //That is the right answer for most things and wrong for the rest: a sprite carrying a lot of empty pixels
        //around it, or artwork drawn much bigger than the part that should actually be hittable. Switching this on
        //ignores the artwork entirely and uses ManualBoundsSize instead, centred on the owner's Position - which is
        //the point its Origin is pinned to, so the box sits on the pivot no matter where the sprite ended up
        //around it.
        //
        //CollisionRadius still expands the result, manual or not, because everything downstream expects the one
        //pipeline. Set it to 0 if the numbers below are meant to be the final box. With ShowCollisionBounds on the
        //white outline is what this produced and the coloured one is after the radius, so the two are visible apart
        bool UseManualBounds = false;

        //Full width and height of the manual box, not half extents. Only read while UseManualBounds is set, and
        //clamped at zero on the way out - a negative size would be a box nothing can ever intersect
        ETG::Vector2f ManualBoundsSize{16.f, 16.f};

        //Where the box sits relative to where it would otherwise land, in world pixels: +x right, +y down. Sizing
        //a box by hand answers "how big is the body", this answers "and where on the sprite is it" - the two are
        //separate questions and a pivot at the feet, at the head or anywhere off centre makes them different
        //answers. Shrinking ManualBoundsSize alone always eats the box symmetrically around Position, so a hero
        //whose Origin is not at his middle can never get a box on his actual body without this.
        //
        //Applied to the drawn bounds too, not only the manual ones - an animation whose frames carry uneven
        //padding is the same problem with a different cause, and there is no reason the fix should be limited to
        //boxes that were typed by hand. Zero by default, so anything that does not set it is untouched.
        //CollisionRadius still expands whatever comes out of here, exactly as before
        ETG::Vector2f BoundsOffset{0.f, 0.f};

        //Whether to show collision bounds for debugging
        bool ShowCollisionBounds = false;

        //Color for collision visualiztion
        ETG::Color CollisionVisualizationColor = ETG::Color::Yellow;

        //Outline width in WORLD pixels, not screen ones - the main view magnifies 5x, so 1.f here lands as five
        //pixels on screen. It used to be impossible to change at all: the old immediate path threw the thickness
        //away and drew a GPU line strip, which is one screen pixel wide however far in the world you are zoomed.
        //Drawn inwards from the bounds, so the rectangle keeps marking the box's true extent however thick the
        //stroke gets - and sub pixel values like 0.5f are fine when a 12x18 body starts looking swallowed by it
        float CollisionVisualizationThickness = 1.f;
        
        std::string Name{}; 

        //Get the current collision bounds (including radius)
        ETG::FloatRect GetCollisionBounds() const {return ExpandedBounds;};

        //Events for collision
        EventDelegate<CollisionEventData> OnCollisionEnter;
        EventDelegate<CollisionEventData> OnCollisionStay;
        EventDelegate<CollisionEventData> OnCollisionExit;

        //Draw current object, radius expanded borders, impact point, line between collided object's center points.
        //
        //Nothing here reaches the screen at the moment it is called any more: it all goes into GlobSpriteBatch,
        //which is what makes the depth below mean anything. Visualize still runs from inside the owner's Draw(),
        //and the batch does not flush until every object has drawn - so an immediate draw here was guaranteed to
        //end up underneath the whole world, however far in front the debug box was supposed to be. The window is
        //still taken because every call site passes one and the signature is not worth churning
        void Visualize(ETG::RenderWindow& window);

        //Get collision registry (all active collision components)
        static SafeRegistry<CollisionComponent>& GetAllCollRegistries() {return AllCollisionRegistries;}

        void SetCollisionEnabled(bool enabled);
        bool IsCollisionEnabled() const { return CollisionEnabled; }

    private:
        //The sweep lives there now, and it needs the two contact lists, the bounds and the narrow-phase test that
        //used to be this class's own business. A friend rather than a set of public getters on purpose: this is
        //not state anybody else has any reason to touch, and naming the one exception says so
        friend class CollisionSystem;

        //Cache the owner's bounds + radius
        ETG::FloatRect ExpandedBounds;

        //collision’ın devam mı ettiğini yoksa bittiğini mi anlamak için kullanılacak.
        //- DispatchExits: Geçen frame vardı ama bu frame yok mu (Exit)? diye bakacak
        //
        SafeRegistry<CollisionComponent> PrevFrameCollisions;

        //Scratch list Update fills each frame, then swaps into CurrentCollisions. A member and not a function
        //local so its capacity survives the frame; a member and not a static because Broadcast runs inside the
        //walk and a listener touching another component would clobber a shared one
        SafeRegistry<CollisionComponent> StillColliding;

        //Registry of all active collision components. To see the owner of any element look at: otherComp->ComponentBase->GameObjectBase->Owner
        static SafeRegistry<CollisionComponent> AllCollisionRegistries;

        //Whether this collision component is active
        bool CollisionEnabled{true};
        bool DrawCollisionLineBetweenCenters{};
        bool DrawImpactPoint{true};

        //The box before CollisionRadius is applied: the owner's drawn bounds, or the manual one when
        //UseManualBounds is set, shifted by BoundsOffset either way. Both places that used to ask Owner->GetBounds()
        //directly go through here, so the visualiser cannot end up drawing a different rectangle from the one the
        //sweep is testing
        [[nodiscard]] ETG::FloatRect GetBaseBounds() const;

        //In Update before starting collision check, update the Owner's bounds including CollisionRadius
        void UpdateBounds();

        //Check colision with another component. Hands back the overlapping region too, because Rect::intersects
        //works it out either way and the caller wants its centre for the impact point. Only filled when it
        //returns true; on false the overlap is zeroed
        bool CheckCollision(const CollisionComponent* other, ETG::FloatRect& outOverlap) const;

        //Centre of the overlap with another component, recomputed from scratch. Update does not use this - it
        //already has the overlap from CheckCollision - so this is left for the two callers that hold no overlap
        //of their own: the debug visualiser and SetCollisionEnabled
        ETG::Vector2f CalculateImpactPoint(const CollisionComponent* other) const;
        void DrawCollisionLineBetweenCenter(const CollisionComponent* otherComp) const;

        //Depth the whole visualization is queued at. The batch sorts descending and draws in that order, so the
        //most negative depth is the last thing painted and therefore the topmost - a debug box that a wall can
        //hide is a debug box that lies to you exactly when you most need it. Matches the default DrawDebugCross
        //already uses for the same reason; nothing in the game draws anywhere near this far forward
        static constexpr float VisualizationDepth = -1000.f;

        BOOST_DESCRIBE_CLASS(CollisionComponent, (ComponentBase), (CollisionEnabled, ShowCollisionBounds, CollisionRadius, UseManualBounds, ManualBoundsSize, BoundsOffset, CollisionVisualizationThickness, DrawImpactPoint,DrawCollisionLineBetweenCenters, CollisionVisualizationColor), (), ())
    };

    struct CollisionEventData
    {
        GameObjectBase* Self = nullptr; //The object that owns this collision component. NOTE: NOT THIS POINTER
        GameObjectBase* Other = nullptr; //The object that collided with this one 
        CollisionComponent* OtherComp = nullptr; //The collision component of the other objects
        ETG::Vector2f ImpactPoint;

        CollisionEventData(GameObjectBase* self, GameObjectBase* other, CollisionComponent* otherComp, const ETG::Vector2f impactPoint) : Self(self), Other(other), OtherComp(otherComp), ImpactPoint(impactPoint)
        {
        }
    };
}
