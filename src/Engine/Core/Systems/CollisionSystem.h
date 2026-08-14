#pragma once
#include <vector>
#include "../../Platform/Vector2.h"

// Eskiden her collider kendi taramasını sahibinin Update()'i içinden yapıyordu (CollisionComponent::Update → tüm registry'yi gez,
// herkesin bounds'unu oku, kendi event'lerini at). Sorun şu ki nesneler aynı anda hareket etmiyor — her biri kendi Update'inde hareket ediyor.
// Üstelik kendi içlerinde bile sıraları tutarlı değildi: Yani tek bir çift için bir taraf çakışmayı görürken diğeri aynı frame'de görmeyebiliyordu; sonuç kimin önce çalıştığına,
// o da nesnelerin spawn sırasına bağlıydı. Kimsenin verdiği bir karar değildi.

// Çözüm: frame'i ikiye bölmek
//
// GameManager::Update'te önce herkes hareket eder, sonra CollisionSystem::Update() bir kez çalışır. Yaptığı her test pozisyonları aynı tek andan okur —
// kimse "erken" ya da "geç" olamaz, çünkü artık kimse kendi adına test yapmıyor.

namespace ETG
{
    class CollisionComponent;

    //THE ONE PLACE COLLISION IS RESOLVED
    //
    //THE PROBLEM THIS EXISTS FOR
    //
    //Every collider used to run its own pass from inside its owner's Update: it walked the whole registry, read
    //everybody else's bounds and fired its own events, right there in the middle of the frame. That means a
    //collider read its neighbours at whatever position they happened to be holding at that instant, and objects do
    //not move at the same instant - each one moves in its own Update. Worse, they did not even agree on the order
    //within themselves: Hero tested collision before it moved, Enemy after, Projectile before. So for one pair,
    //one side could see an overlap and the other side not see it, in the same frame, purely because of who ran
    //first. The order itself was nobody's decision - it fell out of the order objects happened to be spawned in.
    //
    //THE FIX
    //
    //Split the frame. Everything moves first (that is the object loop in GameManager::Update); then this runs,
    //once, and every test it performs reads positions from that same single instant. Nobody can be "early" or
    //"late" any more, because nobody is testing anything on their own behalf.
    //
    //THE FIVE PHASES, AND WHY THEY ARE SEPARATE
    //
    //  1 CollectActive  - snapshot the colliders taking part, refresh all their bounds
    //  2 DetectContacts - every pair, once, pure reading. Not a single event is fired here
    //  3 Enter / Stay   - now the events go out, from a contact list that is already final
    //  4 Exit           - everything that was a contact last frame and is not one now
    //  5 Commit         - this frame's contacts become the record the next frame compares against
    //
    //Phase 2 is deliberately allowed to touch nothing but bounds. The moment an event is broadcast, game code runs
    //and it may do anything - damage, destroy, switch a collider off - so if detection and dispatch were
    //interleaved (which is exactly what the old per-component pass did) the second half of the sweep would be
    //testing against a world the first half had already changed. Keeping them apart is also what makes the
    //detection loop the only part that could ever be handed to more than one thread, should it ever be worth it:
    //it reads, and it writes nothing anybody else is reading.
    class CollisionSystem
    {
    public:
        //Runs the whole collision frame. Call exactly once, after every object has moved and before the frame's
        //destroyed objects are swept
        static void Update();

        //A collider being destroyed has to drop out of any contact this frame's dispatch has not reached yet -
        //otherwise the dispatch reads bounds out of freed memory. Called from ~CollisionComponent, alongside the
        //scrub it already does of everybody's contact lists
        static void ForgetComponent(const CollisionComponent* component);

    private:
        //One side of one overlap: the event that Self is about to be told about. Masks may be one-way (a gun on
        //the floor watches for the hero, the hero does not watch for guns - see CollisionComponent.h), so a pair
        //produces one of these per direction that is actually interested, each with its own point of view
        struct Contact
        {
            CollisionComponent* Self;
            CollisionComponent* Other;
            ETG::Vector2f ImpactPoint;
        };

        static void CollectActive();
        static void DetectContacts();
        static void DispatchEnterAndStay();
        static void DispatchExits();
        static void Commit();

        //Both buffers are members rather than locals so the capacity they grow into survives the frame: refilled
        //every tick, they stop allocating after the first few. Both are also blanked in place by ForgetComponent
        //rather than erased from, so an index a phase is holding never stops meaning what it meant
        static inline std::vector<CollisionComponent*> Active{};
        static inline std::vector<Contact> Contacts{};
    };
}
