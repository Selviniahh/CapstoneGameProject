#include <algorithm>
#include <imgui.h>
#include "CollisionComponent.h"
#include "../../Core/GameObjectBase.h"
#include "../../Core/Systems/CollisionSystem.h"
#include "../../Managers/RenderContext.h"
#include "../../Managers/SpriteBatch.h"

namespace ETG
{
    SafeRegistry<CollisionComponent> CollisionComponent::AllCollisionRegistries;

    CollisionComponent::CollisionComponent()
    {
        AllCollisionRegistries.Add(this);
    }

    CollisionComponent::~CollisionComponent()
    {
        AllCollisionRegistries.Remove(this);

        //Anyone still holding us as a contact has to let go: next frame they would be reading bounds out of freed
        //memory. SafeRegistry decides on its own whether it is safe to really erase or has to blank the slot,
        //which matters here because this destructor often runs from inside somebody else's walk
        AllCollisionRegistries.ForEach([this](CollisionComponent* component)
        {
            component->PrevFrameCollisions.Remove(this);
            component->StillColliding.Remove(this);
        });

        //And the sweep in progress has to let go too. Its contact list is built before any event goes out, so a
        //listener that destroys us leaves our pointer sitting in contacts that have not been dispatched yet
        CollisionSystem::ForgetComponent(this);
    }

    void CollisionComponent::Initialize()
    {
        ComponentBase::Initialize();
        UpdateBounds();
    }

    

    ETG::FloatRect CollisionComponent::GetBaseBounds() const
    {
        if (!Owner) return {};

        //The offset is a plain translation, so it lands the same way on both branches: slide the finished
        //rectangle, never resize it. Sizing and placing stay two separate knobs that cannot disturb each other
        if (!UseManualBounds)
        {
            ETG::FloatRect drawn = Owner->GetBounds();
            drawn.left += BoundsOffset.x;
            drawn.top += BoundsOffset.y;
            return drawn;
        }

        //Position is where the owner's Origin lands, so centring on it puts the box on the pivot rather than on
        //wherever the artwork happens to sit around that pivot - which is the whole reason to type a size by hand.
        //BoundsOffset then moves that centre off the pivot deliberately, for bodies the pivot is not in the middle of
        const ETG::Vector2f center = Owner->GetPosition() + BoundsOffset;

        //Clamped because a negative size reads as inverted to Rect::intersects, and an inverted rect silently
        //never collides. Zero is the same "never collides" but at least it draws as nothing rather than as junk
        const float width = std::max(ManualBoundsSize.x, 0.f);
        const float height = std::max(ManualBoundsSize.y, 0.f);

        return {center.x - width / 2.f, center.y - height / 2.f, width, height};
    }

    void CollisionComponent::UpdateBounds()
    {
        if (!Owner) return;

        // Get basic bounds from owner that not expanded yet.
        const ETG::FloatRect baseBounds = GetBaseBounds();

        // Expand by radius
        ExpandedBounds = ETG::FloatRect(
            baseBounds.left - CollisionRadius,
            baseBounds.top - CollisionRadius,
            baseBounds.width + (2 * CollisionRadius),
            baseBounds.height + (2 * CollisionRadius)
        );
    }

    bool CollisionComponent::CheckCollision(const CollisionComponent* other, ETG::FloatRect& outOverlap) const
    {
        //The message used to be built by dereferencing the very pointer it was checking for null
        if (!other) throw std::runtime_error("CheckCollision on " + Owner->ObjectName + " was given a null component");

        //Thankfully at least I am not have to implement intersection this time myself. The two argument overload
        //is what the plain one calls anyway, discarding the overlap into a dummy - so keeping it is free
        return ExpandedBounds.intersects(other->GetCollisionBounds(), outOverlap);
    }

    ETG::Vector2f CollisionComponent::CalculateImpactPoint(const CollisionComponent* other) const
    {
        ETG::FloatRect intersection;

        if (ExpandedBounds.intersects(other->GetCollisionBounds(), intersection))
            return intersection.getCenter();

        return {0, 0};
    }

    void CollisionComponent::Visualize(ETG::RenderWindow& window)
    {
        if (!ShowCollisionBounds || !CollisionEnabled || !Owner || !Owner->IsVisible) return;

        // Draw the original bounds and expanded bounds. The base one comes from GetBaseBounds and not from the
        // owner, so that with UseManualBounds on the white outline is the hand typed box rather than the artwork
        // it is standing in for
        const ETG::FloatRect baseBounds = GetBaseBounds();

        //Both rectangles are queued into the batch at the same depth, so the one queued second draws over the
        //first - the expanded box on top of the base one, which is the order they came out in before as well
        GlobSpriteBatch.drawRectOutline(baseBounds, ETG::Color::White, CollisionVisualizationThickness, VisualizationDepth);
        GlobSpriteBatch.drawRectOutline(ExpandedBounds, CollisionVisualizationColor, CollisionVisualizationThickness, VisualizationDepth);

        // Visualize current collisions
        PrevFrameCollisions.ForEach([this](const CollisionComponent* otherComp)
        {
            if (!otherComp->Owner) return;

            if (DrawCollisionLineBetweenCenters)
            {
                DrawCollisionLineBetweenCenter(otherComp);
            }

            if (DrawImpactPoint)
            {
                //A cross where there used to be a circle, for the same reason the boxes moved: only the batch can
                //put a marker in front of the sprites it is marking, and the batch draws textured quads. The cross
                //is the one debug marker already built out of those, and it is drawn at the same overlay depth
                const ETG::Vector2f impactPoint = CalculateImpactPoint(otherComp);
                if (impactPoint != ETG::Vector2f{0, 0})
                    SpriteBatch::DrawDebugCross(impactPoint, ETG::Color::Green, 3.f, VisualizationDepth);
            }
        });
    }

    void CollisionComponent::DrawCollisionLineBetweenCenter(const CollisionComponent* otherComp) const
    {
        // Draw a line connecting the centers
        ETG::Vector2f selfCenter(
            ExpandedBounds.left + ExpandedBounds.width / 2,
            ExpandedBounds.top + ExpandedBounds.height / 2
        );

        ETG::FloatRect otherBounds = otherComp->GetCollisionBounds();
        ETG::Vector2f otherCenter(
            otherBounds.left + otherBounds.width / 2,
            otherBounds.top + otherBounds.height / 2
        );

        SpriteBatch::DrawDebugLine(selfCenter, otherCenter, ETG::Color::Red, CollisionVisualizationThickness, VisualizationDepth);
    }

    //TODO: I am not sure if I should remove this function. For now let's put it bottom of this class to ignore easier 
    void CollisionComponent::SetCollisionEnabled(const bool enabled)
    {
        if (CollisionEnabled == enabled) return;

        CollisionEnabled = enabled;

        // Clear current collisions if disabling
        if (!enabled)
        {
            // Notify exit events for all current collisions. Unlike Update's sweep the two objects are still
            // overlapping here - we are switching off, not moving apart - so there is a real impact point to give
            PrevFrameCollisions.ForEach([this](CollisionComponent* otherComp)
            {
                if (!otherComp->Owner) return;

                const CollisionEventData eventData(Owner, otherComp->Owner, otherComp, CalculateImpactPoint(otherComp));
                OnCollisionExit.Broadcast(eventData);
            });

            PrevFrameCollisions.Clear();
        }
    }
}
