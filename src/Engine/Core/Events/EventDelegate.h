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
        using Callback = std::function<void(Args...)>;
        using ListenerHandle = size_t;
        constexpr static ListenerHandle InvalidHandle = -1;

        // Add a listener and return a handle
        ListenerHandle AddListener(Callback callBack)
        {
            ListenerHandle handle = nextHandle++;
            Listeners.emplace(handle, std::move(callBack));
            return handle;
        }

        // Remove a specific listener using its handle
        void RemoveListener(ListenerHandle handle)
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
        std::map<ListenerHandle, Callback> Listeners;
        ListenerHandle nextHandle = 0;
    };
}