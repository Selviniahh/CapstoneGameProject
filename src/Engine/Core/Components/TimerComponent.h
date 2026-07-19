#pragma once
#include "../GameClass.h"
#include "../Events/EventDelegate.h"
#include "../../Managers/Globals.h"
#include "../ComponentBase.h"

namespace ETG
{
    class TimerComponent : public ComponentBase 
    {
    public:
        // Constructor that takes total time in seconds
        explicit TimerComponent(float totalTimeSeconds);
        ~TimerComponent() override;

        // Update function to be called every frame
        void Update() override;

        // Start/Stop/Reset controls
        void Start();
        void Stop();
        void Reset();
        void Restart(); // Combines Reset() and Start()

        // Check if timer is up
        [[nodiscard]] bool IsFinished() const;

        // Get remaining and elapsed time
        [[nodiscard]] float GetRemainingTime() const;
        [[nodiscard]] float GetElapsedTime() const;

        // Get percentage complete (0.0 to 1.0)
        [[nodiscard]] float GetProgress() const;

        // Set a new duration
        void SetDuration(float newTotalTime);

        // Delegate that broadcasts when timer completes
        EventDelegate<> OnTimerFinished;


    private:
        float TotalTime; // Total time in seconds
        float CurrentTime; // Current elapsed time. This will be incremented every frame
        bool bIsRunning; // Is the timer currently running?
        bool bIsFinished; // Has the timer completed?

        BOOST_DESCRIBE_CLASS(TimerComponent, (GameClass),
                             (TotalTime, bIsRunning, bIsFinished),
                             (), ())
    };
}
