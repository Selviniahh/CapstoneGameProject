#pragma once

namespace ETG::Time
{
    //Seconds elapsed between the last two frames. AKA Delta Time
    extern float FrameTick;

    //Start the tick clock. Call once before the first frame.
    void Initialize();

    //Advance the tick clock; call once at the beginning of every frame.
    void Update();

    //Restart the tick clock. Call when resuming after a pause (e.g. focus regained) so the
    //paused duration doesn't land on the next frame as one giant FrameTick.
    void ResetTick();
}
