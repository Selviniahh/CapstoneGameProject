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

    //An object sits on exactly one layer and carries a Mask of the layers it wants to hear about. Everything else
    //is rejected before the bounds are ever touched, which is what keeps a screen full of bullets from testing
    //every bullet against every other one.
    //
    //The pairing is deliberately allowed to be one-way: a gun waiting on the floor watches for the hero, while
    //the hero does not watch for guns, because picking it up is the gun's listener to run and nothing on the
    //hero's side needs to know. The cost of that is the rule to remember - a listener for something your Mask
    //does not include will never fire. Widen the Mask in the same commit that adds the listener
    
    //    CollisionComp->Mask = CollisionLayer::Enemy | CollisionLayer::Projectile; 
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

        //Deliberately does nothing. Collision is not something a collider does to the world on its own schedule
        //any more - CollisionSystem::Update resolves every collider at once, from GameManager::Update, after
        //everything has finished moving. Left in place, and left empty, so that an owner still calling this out of
        //habit is harmless rather than a second sweep running at the wrong moment
        void Update() override;

        //Which layer this object is. Exactly one bit
        uint32_t Layer = CollisionLayer::Default;

        //Which layers this object wants to be told about. Defaults to everything so a component that never sets
        //it keeps the old behaviour: slower, but never silently missing a collision
        
        uint32_t Mask = CollisionLayer::All;

        //Radius to expand collision box beyond the texture boundaries
        float CollisionRadius = 1.0f;

        //Whether to show collision bounds for debugging
        bool ShowCollisionBounds = false;

        //Color for collision visualiztion
        ETG::Color CollisionVisualizationColor = ETG::Color::Yellow;

        //Get the current collision bounds (including radius)
        ETG::FloatRect GetCollisionBounds() const {return ExpandedBounds;};

        //Events for collision
        EventDelegate<CollisionEventData> OnCollisionEnter;
        EventDelegate<CollisionEventData> OnCollisionStay;
        EventDelegate<CollisionEventData> OnCollisionExit;

        //Draw current object, radius expanded borders, impact point, line between collided object's center points.
        void Visualize(ETG::RenderWindow& window);

        //Get collision registry (all active collision components)
        static SafeRegistry<CollisionComponent>& GetRegistry() {return AllCollisionRegistries;}

        void SetCollisionEnabled(bool enabled);
        bool IsCollisionEnabled() const { return CollisionEnabled; }

    private:
        //The sweep lives there now, and it needs the two contact lists, the bounds and the narrow-phase test that
        //used to be this class's own business. A friend rather than a set of public getters on purpose: this is
        //not state anybody else has any reason to touch, and naming the one exception says so
        friend class CollisionSystem;

        //Cache the owner's bounds + radius
        ETG::FloatRect ExpandedBounds;

        //Hold which objects we are currently colliding with. A list, not a map: an object touches 0-3 things at
        //once, and at that size a linear scan beats hashing. The two lists trade buffers through SwapWith in
        //Update, so after the first few frames neither one allocates again
        SafeRegistry<CollisionComponent> CurrentCollisions;

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
        void DrawCollisionLineBetweenCenter(ETG::RenderWindow& window, const CollisionComponent* otherComp) const;

        BOOST_DESCRIBE_CLASS(CollisionComponent, (ComponentBase), (CollisionEnabled, ShowCollisionBounds, CollisionRadius,DrawImpactPoint,DrawCollisionLineBetweenCenters, CollisionVisualizationColor), (), ())
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
