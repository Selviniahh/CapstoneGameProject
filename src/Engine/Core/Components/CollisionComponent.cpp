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
            component->CurrentCollisions.Remove(this);
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

    //Empty on purpose - see the declaration. The sweep that used to live here is CollisionSystem::Update, which
    //runs once for every collider after the world has finished moving. A collider testing the world from inside
    //its owner's Update is precisely the thing that made the result depend on who was updated first
    void CollisionComponent::Update()
    {
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
            return intersection.getCenter();

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
