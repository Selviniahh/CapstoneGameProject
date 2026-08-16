#pragma once
#include <algorithm>
#include <vector>

namespace ETG
{
    //A list of pointers that stays safe to walk while the walk itself is changing it.
    //
    //THE PROBLEM
    //
    //Collision code walks a list of components and, for every hit it finds, calls into game code through an
    //event. That game code is allowed to do anything, including destroying an object - which takes that object's
    //component out of the very list being walked. Erasing from a std::vector shifts every later element down and
    //may move the whole buffer to a different address, so the position the loop was holding stops meaning what it
    //meant. Reading it afterwards is undefined behaviour: usually a crash, sometimes worse - a wrong answer that
    //passes silently and brings the program down three frames later somewhere unrelated.
    //
    //THE FIX
    //
    //Never restructure the list while anyone is inside ForEach. A removal during a walk blanks that slot instead
    //of erasing it: writing a pointer moves nothing, so every position anyone is holding stays valid. ForEach
    //steps over the blanks, and sweeps them away the next time it runs with nobody else looking.
    //
    //WHY THE DEPTH IS SHARED
    //
    //WalkDepth is one counter for every SafeRegistry of the same T, not one per list. It has to be: while the
    //master list of components is being walked, the small per-component lists must not restructure either,
    //because the same event reaches them too. One event, one rule - nothing moves until every walk is out.
    //
    //A counter rather than a flag, because walks nest: an event fired from inside one walk can start another.
    //Only the outermost one leaving means it is safe to move things again.
    
 // - Sadece elemanın alanları değişiyor → normal for
 // - Container hiç değişmiyor → normal for
 // - Başka, ilgisiz bir container değişiyor → normal for
 // - Dolaşılan container’a eleman ekleniyor/siliniyor → SafeRegistry
 // - Callback/event çağrılıyor ve ne yapabileceği belirsiz → SafeRegistry kullanmak mantıklı
 // - Nesneler kendilerini destructor’da registry’den çıkarıyorsa ve döngü sırasında yok
 //   edilebiliyorsa → SafeRegistry
    template <typename T>
    class SafeRegistry
    {
    public:
        void Add(T* item)
        {
            Items.push_back(item);
        }

        void Remove(T* item)
        {
            if (WalkInProgress())
            {
                //Blank it instead of erasing. Whoever is walking keeps their place and ForEach will step over it
                std::ranges::replace(Items, item, static_cast<T*>(nullptr));
                HasBlanks = true;
                return;
            }

            std::erase(Items, item);
        }

        [[nodiscard]] bool Contains(const T* item) const
        {
            return std::ranges::find(Items, item) != Items.end();
        }

        //Empties the list but keeps the memory it already grew into, so a list refilled every frame stops
        //allocating after the first few
        void Clear()
        {
            Items.clear();
            HasBlanks = false;
        }

        //Trades buffers with another list rather than copying between them. Two lists that swap back and forth
        //every frame reuse the same two allocations forever
        void SwapWith(SafeRegistry& other)
        {
            Items.swap(other.Items);
            std::swap(HasBlanks, other.HasBlanks);
        }

        [[nodiscard]] bool IsEmpty() const { return Items.empty(); }

        //Runs body(item) for every live entry. Blanks are skipped, so body never sees a null.
        //
        //Inside body you may remove from this list, add to it, or destroy something that removes itself - all of
        //it is safe. What you may not do is expect a removal to take effect immediately: it becomes a blank now
        //and is really gone once the walk ends
        //NOTE:  bir array parametresi almak zorunda deĞil. Verilen type'ın memberlarından bir array'i alabilir 
        template <typename Body>
        void ForEach(Body&& body)
        {
            //Birşey nullptr sa SweepBlanks den temizle ki sonra iterator bir başladığında bozulmasın
            SweepBlanks();

            const WalkScope scope;

            //Indexed, and size() re-read every step, because body is allowed to Add: a push_back that reallocates
            //would leave an iterator pointing into freed memory, while an index simply keeps counting. Something
            //added mid-walk is picked up by this same walk, which is what we want - it exists, so it collides
            for (size_t i = 0; i < Items.size(); ++i)
            {
                T* item = Items[i];

                //Blanked during this walk by something body destroyed
                if (!item) continue;

                body(item);
            }
        }

        static bool WalkInProgress() { return WalkDepth > 0; } //walkin dolaşım demek 

    private:
        //Only ever called with no walk running, so the erase is free to move things
        void SweepBlanks()
        {
            //Sadece Blank varsa devam et ve nullptr olanları kaldır 
            if (!HasBlanks || WalkInProgress()) return;

            std::erase(Items, nullptr);
            HasBlanks = false;
        }

        //Counts a walk in for as long as it lives. Written as a constructor/destructor pair rather than a ++ and
        //a -- around the loop so that the -- cannot be skipped: it still runs if the loop returns early, and it
        //still runs if something inside throws. Copying is deleted because a copy would destruct a second time
        //and push the count below zero, at which point the guard silently stops guarding
        struct WalkScope
        {
            WalkScope() { ++WalkDepth; }
            ~WalkScope() { --WalkDepth; }

            WalkScope(const WalkScope&) = delete;
            WalkScope& operator=(const WalkScope&) = delete;
        };

        std::vector<T*> Items;

        //Whether this list is carrying blanks left by a removal during a walk
        //blank, listeden bir eleman silindiğinde onun yerinde kalan boş yuva/boşluk
        bool HasBlanks = false;

        //Shared by every SafeRegistry<T>. See WHY THE DEPTH IS SHARED above
        static inline int WalkDepth = 0;
    };
}
