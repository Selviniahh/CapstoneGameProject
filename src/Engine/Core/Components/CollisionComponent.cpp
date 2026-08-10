#include <imgui.h>
#include "CollisionComponent.h"
#include "../../Core/GameObjectBase.h"
#include "../../Managers/RenderContext.h"
#include "../../Managers/SpriteBatch.h"

namespace ETG
{
    namespace
    {
        //Where two bounds meet, reported as the middle of the region they share
        ETG::Vector2f CentreOf(const ETG::FloatRect& rect)
        {
            return {
                rect.left + rect.width / 2.0f, //x
                rect.top + rect.height / 2.0f //y
            };
        }

    }

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
            component->CurrentCollisions.Remove(this);
            component->StillColliding.Remove(this);
        });
    }

    void CollisionComponent::Initialize()
    {
        ComponentBase::Initialize();
        UpdateBounds();
    }

    void CollisionComponent::Update()
    {
        if (!CollisionEnabled || !Owner) return;

        // Update our bounds based on the owner's position and texture
        UpdateBounds();

        // This frame's contacts get collected here, then become CurrentCollisions at the end
        StillColliding.Clear();

        // Who are we touching right now?
        AllCollisionRegistries.ForEach([this](CollisionComponent* otherComp)
        {
            //Skip the unappropriated ones. The layer test comes early because it is by far the most selective and
            //costs one load and one and, where everything past it reads both objects' bounds

            // Bitwise işlemin sonucunun 0 olması:
            //> “Bu collider’ın Mask değeri, diğer collider’ın Layer bitini içermiyor.”
            if (otherComp == this || !(Mask & otherComp->Layer) || !otherComp->IsCollisionEnabled() || !otherComp->Owner)
                return;

            //The overlap comes back from the same test that decided whether there is one, so the impact point
            //below is four arithmetic ops on a rect already in hand instead of a second full intersection
            ETG::FloatRect overlap;
            if (!CheckCollision(otherComp, overlap)) return;

            const bool wasTouchingLastFrame = CurrentCollisions.Contains(otherComp);

            StillColliding.Add(otherComp);

            const CollisionEventData eventData(Owner, otherComp->Owner, otherComp, CentreOf(overlap));

            if (!wasTouchingLastFrame)
                OnCollisionEnter.Broadcast(eventData);
            else
                OnCollisionStay.Broadcast(eventData);

            //No exit is raised here on purpose. The sweep below already reports everything that left
            //CurrentCollisions and it catches strictly more: a component that stopped intersecting, but equally
            //one that was skipped this frame because it disabled its collision or fell out of the Mask. Raising
            //it in both places is how this used to fire OnCollisionExit twice for a single separation
        });

        // Who were we touching last frame but are not any more?
        CurrentCollisions.ForEach([this](CollisionComponent* otherComp)
        {
            if (!otherComp->Owner || StillColliding.Contains(otherComp)) return;

            // The two no longer overlap, so there is no impact point to report
            const CollisionEventData eventData(Owner, otherComp->Owner, otherComp, ETG::Vector2f{0, 0});
            OnCollisionExit.Broadcast(eventData);
        });

        // This frame's contacts become the record to compare against next frame. A swap and not a copy: the two
        // lists hand buffers back and forth, so neither allocates again after the first few frames
        CurrentCollisions.SwapWith(StillColliding);
    }

    void CollisionComponent::UpdateBounds()
    {
        if (!Owner) return;

        // Get basic bounds from owner that not expanded yet. 
        const ETG::FloatRect baseBounds = Owner->GetBounds();

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
            return CentreOf(intersection);

        return {0, 0};
    }

    void CollisionComponent::Visualize(ETG::RenderWindow& window)
    {
        if (!ShowCollisionBounds || !CollisionEnabled || !Owner || !Owner->IsVisible) return;

        // Draw the original bounds and expanded bounds
        ETG::FloatRect baseBounds = Owner->GetBounds();

        // Original bounds in white
        ETG::RectangleShape baseRect;
        baseRect.setPosition(baseBounds.left, baseBounds.top);
        baseRect.setSize(ETG::Vector2f(baseBounds.width, baseBounds.height));
        baseRect.setFillColor(ETG::Color::Transparent);
        baseRect.setOutlineColor(ETG::Color::White);
        baseRect.setOutlineThickness(1.0f);
        window.draw(baseRect);

        // Expanded bounds in configured color
        ETG::RectangleShape expandedRect;
        expandedRect.setPosition(ExpandedBounds.left, ExpandedBounds.top);
        expandedRect.setSize(ETG::Vector2f(ExpandedBounds.width, ExpandedBounds.height));
        expandedRect.setFillColor(ETG::Color::Transparent);
        expandedRect.setOutlineColor(CollisionVisualizationColor);
        expandedRect.setOutlineThickness(1.0f);
        window.draw(expandedRect);

        // Visualize current collisions
        CurrentCollisions.ForEach([this, &window](const CollisionComponent* otherComp)
        {
            if (!otherComp->Owner) return;

            if (DrawCollisionLineBetweenCenters)
            {
                DrawCollisionLineBetweenCenter(window, otherComp);
            }

            if (DrawImpactPoint)
            {
                ETG::CircleShape circle;
                circle.setOrigin(5.0f, 5.0f);
                circle.setRadius(5);
                circle.setPosition(CalculateImpactPoint(otherComp));
                circle.setFillColor(ETG::Color::Green);
                if (circle.getPosition() != ETG::Vector2f{0, 0})
                    RenderContext::Window->draw(circle);
            }
        });
    }

    void CollisionComponent::DrawCollisionLineBetweenCenter(ETG::RenderWindow& window, const CollisionComponent* otherComp) const
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

        window.drawLine(selfCenter, otherCenter, ETG::Color::Red);
    }

    SafeRegistry<CollisionComponent>& CollisionComponent::GetRegistry()
    {
        return AllCollisionRegistries;
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
            CurrentCollisions.ForEach([this](CollisionComponent* otherComp)
            {
                if (!otherComp->Owner) return;

                const CollisionEventData eventData(Owner, otherComp->Owner, otherComp, CalculateImpactPoint(otherComp));
                OnCollisionExit.Broadcast(eventData);
            });

            CurrentCollisions.Clear();
        }
    }
}
