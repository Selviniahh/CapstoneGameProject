#include "TimerComponent.h"

namespace ETG
{
    TimerComponent::TimerComponent(const float totalTimeSeconds) : TotalTime(totalTimeSeconds), CurrentTime(0.0f), bIsRunning(false), bIsFinished(false) {}

    TimerComponent::~TimerComponent() = default;

    void TimerComponent::Update()
    {
        if (!bIsRunning || bIsFinished) return;

        // Increment timer with frame delta time
        CurrentTime += Globals::FrameTick;

        // Check if timer is finished
        if (CurrentTime >= TotalTime)
        {
            CurrentTime = TotalTime;
            bIsFinished = true;
            bIsRunning = false;

            // Broadcast timer finished event
            OnTimerFinished.Broadcast();
        }
    }

    void TimerComponent::Start()
    {
        bIsRunning = true;
    }

    void TimerComponent::Stop()
    {
        bIsRunning = false;
    }

    void TimerComponent::Reset()
    {
        CurrentTime = 0.0f;
        bIsFinished = false;
    }

    void TimerComponent::Restart()
    {
        Reset();
        Start();
    }

    bool TimerComponent::IsFinished() const
    {
        return bIsFinished;
    }

    float TimerComponent::GetRemainingTime() const
    {
        return TotalTime - CurrentTime;
    }

    float TimerComponent::GetElapsedTime() const
    {
        return CurrentTime;
    }

    float TimerComponent::GetProgress() const
    {
        return CurrentTime / TotalTime;
    }

    void TimerComponent::SetDuration(const float newTotalTime)
    {
        TotalTime = newTotalTime;

        // If we're already past the new duration, finish immediately
        if (CurrentTime >= TotalTime && bIsRunning)
        {
            bIsFinished = true;
            bIsRunning = false;
            OnTimerFinished.Broadcast();
        }
    }
}
