#pragma once
#include "../ComponentBase.h"
#include "../Events/EventDelegate.h"

namespace ETG
{
    class GameObjectBase;
    class CollisionEventData;

    class CollisionComponent : public ComponentBase
    {
    public:
        CollisionComponent();
        ~CollisionComponent() override;

        void Initialize() override;
        void Update() override;

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
        static std::vector<CollisionComponent*>& GetRegistry();

        void SetCollisionEnabled(bool enabled);
        bool IsCollisionEnabled() const { return CollisionEnabled; }

    private:
        //Cache the owner's bounds + radius
        ETG::FloatRect ExpandedBounds;

        //Hold which objects we are currently colliding with. A vector, not a map: an object touches 0-3 things at
        //once, and at that size a linear scan beats hashing. The two buffers ping-pong through swap() in Update,
        //so after the first few frames neither one allocates again
        std::vector<CollisionComponent*> CurrentCollisions;

        //Scratch buffer Update fills each frame, then swaps into CurrentCollisions. A member and not a function
        //local so its capacity survives the frame; a member and not a static because Broadcast runs inside the
        //loop and a listener touching another component would clobber a shared one
        std::vector<CollisionComponent*> StillColliding;

        //Registry of all active collision components. To see the owner of any element look at: otherComp->ComponentBase->GameObjectBase->Owner 
        static std::vector<CollisionComponent*> AllCollisionRegistries;

        //Whether this collision component is active
        bool CollisionEnabled{true};
        bool DrawCollisionLineBetweenCenters{};
        bool DrawImpactPoint{true};

        //In Update before starting collision check, update the Owner's bounds including CollisionRadius
        void UpdateBounds();

        //Check colision with another component
        bool CheckCollision(const CollisionComponent* other) const;
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
