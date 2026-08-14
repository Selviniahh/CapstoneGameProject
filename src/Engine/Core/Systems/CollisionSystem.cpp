#include "CollisionSystem.h"
#include "../Components/CollisionComponent.h"
#include "../GameObjectBase.h"

namespace ETG
{
    void CollisionSystem::Update()
    {
        CollectActive();
        DetectContacts();
        DispatchEnterAndStay();
        DispatchExits();
        Commit();
    }

    void CollisionSystem::ForgetComponent(const CollisionComponent* component)
    {
        //Blanked, never erased, for the same reason SafeRegistry blanks: a phase below may be standing at an index
        //in one of these lists right now, and erasing would shift everything after it out from under that index.
        //Both dispatch phases step over the blanks
        for (CollisionComponent*& active : Active)
        {
            if (active == component) active = nullptr;
        }

        for (Contact& contact : Contacts)
        {
            if (contact.Self == component || contact.Other == component)
            {
                contact.Self = nullptr;
                contact.Other = nullptr;
            }
        }
    }

    //PHASE 1. Who is taking part, and where is everybody
    void CollisionSystem::CollectActive()
    {
        Active.clear();

        CollisionComponent::GetRegistry().ForEach([](CollisionComponent* component)
        {
            if (!component->IsCollisionEnabled() || !component->Owner) return;

            Active.push_back(component);
        });

        //Deliberately outside the walk above: Clear() restructures a list, and SafeRegistry's rule is that nothing
        //restructures while any walk of the same type is open
        for (CollisionComponent* component : Active)
        {
            //Every collider's bounds are refreshed here, in one pass, before a single test runs below. This is the
            //whole point of the split - every test reads positions from the same instant, so it cannot matter
            //whose Update moved them first
            component->UpdateBounds();
            component->StillColliding.Clear();
        }
    }

    //PHASE 2. Every pair, exactly once. Reads only - no events, so nothing can change underneath us
    void CollisionSystem::DetectContacts()
    {
        Contacts.clear();

        //j starts one past i, so a pair is visited once instead of twice. The old scheme had no choice about that:
        //every component walked the entire registry on its own behalf, so (a,b) and (b,a) were separate sweeps
        for (size_t i = 0; i < Active.size(); ++i)
        {
            CollisionComponent* a = Active[i];

            for (size_t j = i + 1; j < Active.size(); ++j)
            {
                CollisionComponent* b = Active[j];

                //The pair is asked in both directions because the Mask relation is allowed to be one-way, and
                //asking once would silently drop those. Neither side interested means the bounds are never read:
                //this test is two loads and an and, everything past it touches both objects' rectangles
                const bool aWatchesB = (a->Mask & b->Layer) != 0;
                const bool bWatchesA = (b->Mask & a->Layer) != 0;
                if (!aWatchesB && !bWatchesA) continue;

                //The overlap comes back from the same test that decided whether there is one, so the impact point
                //is four arithmetic ops on a rect already in hand instead of a second full intersection
                ETG::FloatRect overlap;
                if (!a->CheckCollision(b, overlap)) continue;

                const ETG::Vector2f impactPoint = overlap.getCenter();

                //Both sides learn about the contact from this one test, which is what makes the result symmetric.
                //Under the old scheme each side discovered it separately, on its own schedule, or not at all
                if (aWatchesB)
                {
                    a->StillColliding.Add(b);
                    Contacts.push_back({a, b, impactPoint});
                }

                if (bWatchesA)
                {
                    b->StillColliding.Add(a);
                    Contacts.push_back({b, a, impactPoint});
                }
            }
        }
    }

    //PHASE 3. The contact list is final before the first listener runs, so what game code does in response cannot
    //change who is reported as touching whom this frame
    void CollisionSystem::DispatchEnterAndStay()
    {
        //Indexed, and size() re-read every step, for the reason SafeRegistry::ForEach is: a listener may destroy an
        //object, and ForgetComponent blanks the contacts naming it while we are standing in this list
        for (size_t i = 0; i < Contacts.size(); ++i)
        {
            //Copied out, because ForgetComponent may blank this very slot from inside the broadcast below
            const Contact contact = Contacts[i];
            if (!contact.Self || !contact.Other) continue;
            if (!contact.Self->Owner || !contact.Other->Owner) continue;

            //A listener earlier in this same dispatch may have switched this side off - a projectile does exactly
            //that the moment it lands. It has already been told its contacts ended, so it hears no more of them.
            //Note that the OTHER side is deliberately not checked: the overlap really happened, and whether the
            //other side has since switched itself off is a matter of who came first in this list. Skipping on that
            //would hand the ordering problem straight back
            if (!contact.Self->IsCollisionEnabled()) continue;

            const CollisionEventData eventData(contact.Self->Owner, contact.Other->Owner, contact.Other, contact.ImpactPoint);

            //Enter or Stay is decided against last frame's record, which Commit has not overwritten yet
            if (contact.Self->CurrentCollisions.Contains(contact.Other))
                contact.Self->OnCollisionStay.Broadcast(eventData);
            else
                contact.Self->OnCollisionEnter.Broadcast(eventData);
        }
    }

    //PHASE 4. Who were we touching last frame but are not any more?
    void CollisionSystem::DispatchExits()
    {
        for (size_t i = 0; i < Active.size(); ++i)
        {
            CollisionComponent* component = Active[i];
            if (!component || !component->Owner) continue;

            //Comparing against StillColliding rather than re-testing the bounds catches strictly more: a collider
            //that stopped overlapping, but equally one that switched itself off this frame, or one whose Mask no
            //longer names the other. None of them made it into StillColliding, so all of them read as gone
            component->CurrentCollisions.ForEach([component](CollisionComponent* other)
            {
                if (!other->Owner || component->StillColliding.Contains(other)) return;

                //The two no longer overlap, so there is no impact point to report
                const CollisionEventData eventData(component->Owner, other->Owner, other, ETG::Vector2f{0, 0});
                component->OnCollisionExit.Broadcast(eventData);
            });
        }
    }

    //PHASE 5. This frame's contacts become the record the next frame compares against
    void CollisionSystem::Commit()
    {
        for (size_t i = 0; i < Active.size(); ++i)
        {
            CollisionComponent* component = Active[i];
            if (!component) continue;

            //Switched off partway through the dispatch above. SetCollisionEnabled has already announced the exits
            //and emptied the record; writing this frame's contacts over that would leave a disabled collider
            //holding a list of things it is "touching", and it would still be holding them on the frame somebody
            //switches it back on - at which point those contacts are old enough that no Enter would ever fire
            if (!component->IsCollisionEnabled())
            {
                component->StillColliding.Clear();
                continue;
            }

            //A swap and not a copy: the two lists hand buffers back and forth, so neither allocates again after
            //the first few frames
            component->CurrentCollisions.SwapWith(component->StillColliding);
        }
    }
}
