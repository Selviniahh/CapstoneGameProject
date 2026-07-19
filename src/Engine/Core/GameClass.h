//NOTE: This is important.
//The game drawable objects such as: Hero, Gun, UI are all derived from GameObjectBase
//The components such as: ComponentBase, AnimationBase, HeroAnimComp etc are components and they derived from ComponentBase. Also ComponentBase derived from GameObjectBase
//However Look at Animation folder. There's Animation and AnimationManager class. They are just utility classes not component. THe Animation component using as member variable.
//Now I need to somehow also expose Animation And AnimationManager's member variables to UI. However the reflection logic rejects everything but GameObjectBase, so reflection will ignore `Animation` and `AnimationManager`
//So GameClass implemented. EVERY BUT EVERY Source file needs to be inherited from this class so that their desired member variables can be processed with reflection

#pragma once
#include <mutex>
#include <boost/describe.hpp>
#include <unordered_set>

class GameClass
{
public:
    GameClass();
    
    virtual ~GameClass();

    template<typename T>
    static bool IsValid(const T* ptr)
    {
        if (!ptr) return false;

        std::lock_guard<std::mutex> lock(ValidPointersMutex);
        return ValidPointers.contains(const_cast<void*>(static_cast<const void*>(ptr)));
    }

private:
    static std::unordered_set<void*> ValidPointers; //It's enough to just store void* because we don't need to access anything. All we need to know is the pointer itself is dangling or not
    static std::mutex ValidPointersMutex; //For thread safety

    BOOST_DESCRIBE_CLASS(GameClass, (), (), (), ())
};