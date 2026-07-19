#include "GameClass.h"

// Define the static members
std::unordered_set<void*> GameClass::ValidPointers;
std::mutex GameClass::ValidPointersMutex;

GameClass::GameClass()
{
    //Register this object as valid when created
    std::lock_guard<std::mutex> lock(ValidPointersMutex);
    ValidPointers.insert(static_cast<void*>(this));
}

GameClass::~GameClass()
{
    //Unregister this object when destroyed
    std::lock_guard<std::mutex> lock(ValidPointersMutex);
    ValidPointers.erase(static_cast<void*>(this));
}
