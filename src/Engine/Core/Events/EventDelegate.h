#pragma once
#include <functional>
#include <vector>
#include <map> // Or unordered_map
#include <atomic> // For thread-safe handle generation if needed, simpler counter otherwise

namespace ETG
{
    template <typename... Args>
    class EventDelegate
    {
    public:
        constexpr static size_t InvalidHandle = -1;

        // Add a listener and return a handle
        size_t AddListener(std::function<void(Args...)> callBack)
        {
            size_t handle = nextHandle++;
            Listeners.emplace(handle, std::move(callBack));
            return handle;
        }

        // Remove a specific listener using its handle
        void RemoveListener(size_t handle)
        {
            Listeners.erase(handle);
        }

        // Broadcast the event to all listeners
        void Broadcast(Args... args)
        {
            for (const auto& pair : Listeners)
            {
                pair.second(args...);
            }
        }

        //Remove all listeners. So far I only used this 
        void Clear()
        {
            Listeners.clear();
        }

    private:
        std::map<size_t, std::function<void(Args...)>> Listeners;
        size_t nextHandle = 0;
    };
}