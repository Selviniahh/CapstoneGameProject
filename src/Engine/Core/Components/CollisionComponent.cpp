#include <imgui.h>
#include <algorithm>
#include "CollisionComponent.h"
#include "../../Core/GameObjectBase.h"
#include "../../Managers/RenderContext.h"
#include "../../Managers/SpriteBatch.h"

namespace ETG
{
    std::vector<CollisionComponent*> CollisionComponent::AllCollisionRegistries;

    CollisionComponent::CollisionComponent()
    {
        AllCollisionRegistries.push_back(this);
    }

    CollisionComponent::~CollisionComponent()
    {
        std::erase(AllCollisionRegistries, this);

        for (CollisionComponent* component : AllCollisionRegistries)
        {
            std::erase(component->CurrentCollisions, this);

            //StillColliding outlives the frame now, so a component destroyed while someone else's Update is
            //mid-loop would otherwise leave a dangling pointer to be swapped in at the end of it
            std::erase(component->StillColliding, this);
        }
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

        // Track which components we're still colliding with. clear() keeps the capacity the buffer already grew to
        StillColliding.clear();

        // Check for collisions with all other collision components
        for (auto* otherComp : AllCollisionRegistries)
        {
            //Skip the unappropriated ones
            if (otherComp == this || !otherComp->IsCollisionEnabled() || !otherComp->Owner)
                continue;

            const bool wasColliding = std::ranges::find(CurrentCollisions, otherComp) != CurrentCollisions.end();
            const bool isColliding = CheckCollision(otherComp);

            if (isColliding)
            {
                StillColliding.push_back(otherComp);

                // Handle collision events
                if (!wasColliding)
                {
                    CollisionEventData eventData(Owner, otherComp->Owner, otherComp, CalculateImpactPoint(otherComp));
                    OnCollisionEnter.Broadcast(eventData);
                }
                else
                {
                    // Last tick collided, now still colliding
                    CollisionEventData eventData(Owner, otherComp->Owner, otherComp, CalculateImpactPoint(otherComp));
                    OnCollisionStay.Broadcast(eventData);
                }
            }
            else if (wasColliding)
            {
                // Collision ended
                CollisionEventData eventData(Owner, otherComp->Owner, otherComp, CalculateImpactPoint(otherComp));
                OnCollisionExit.Broadcast(eventData);
            }
        }

        // Find collisions that ended
        for (auto* otherComp : CurrentCollisions)
        {
            if (std::ranges::find(StillColliding, otherComp) == StillColliding.end())
            {
                // This collision has ended
                if (otherComp && otherComp->Owner)
                {
                    CollisionEventData eventData(Owner, otherComp->Owner, otherComp, CalculateImpactPoint(otherComp));
                    OnCollisionExit.Broadcast(eventData);
                }
            }
        }

        // swap and not move: a move would hand StillColliding's buffer away and leave it to allocate a fresh one
        // next frame, which is the very cost this is here to avoid. Swapping hands the old CurrentCollisions
        // buffer back for the clear() above to reuse, so the two ping-pong and neither allocates again
        CurrentCollisions.swap(StillColliding);
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

    bool CollisionComponent::CheckCollision(const CollisionComponent* other) const
    {
        if (!other) throw std::runtime_error("The object: " + other->GetOwner()->ObjectName + " not found");
        //Thankfully at least I am not have to implement intersection this time myself. 
        return ExpandedBounds.intersects(other->GetCollisionBounds());
    }

    ETG::Vector2f CollisionComponent::CalculateImpactPoint(const CollisionComponent* other) const
    {
        ETG::FloatRect intersection;
        ETG::FloatRect otherObjBounds = other->GetCollisionBounds();

        if (ExpandedBounds.intersects(otherObjBounds, intersection))
        {
            return {
                intersection.left + intersection.width / 2.0f, //x
                intersection.top + intersection.height / 2.0f //y
            };
        }

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
        for (auto* otherComp : CurrentCollisions)
        {
            if (otherComp && otherComp->Owner)
            {
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
            }
        }
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

    std::vector<CollisionComponent*>& CollisionComponent::GetRegistry()
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
            // Notify exit events for all current collisions
            for (auto* otherComp : CurrentCollisions)
            {
                if (otherComp && otherComp->Owner)
                {
                    const CollisionEventData eventData(Owner, otherComp->Owner, otherComp, CalculateImpactPoint(otherComp));
                    OnCollisionExit.Broadcast(eventData);
                }
            }

            CurrentCollisions.clear();
        }
    }
}
