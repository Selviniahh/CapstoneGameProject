#include "Time.h"
#include <chrono>

namespace ETG::Time
{
    float FrameTick = 0.0f;

    using Clock = std::chrono::steady_clock;
    static Clock::time_point LastTickTime;

    void Initialize()
    {
        LastTickTime = Clock::now();
    }

    void Update()
    {
        const auto now = Clock::now();

        //Calculate tick. In 60fps it should be: 0.016
        FrameTick = std::chrono::duration<float>(now - LastTickTime).count();
        LastTickTime = now;
    }

    void ResetTick()
    {
        LastTickTime = Clock::now();
    }
}
