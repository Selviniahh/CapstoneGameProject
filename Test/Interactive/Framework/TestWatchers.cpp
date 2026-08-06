#include "TestWatchers.h"
#include <cmath>
#include <cstdio>
#include "Engine/Core/GameObjectBase.h"
#include "Engine/Managers/Time.h"
#include "Utils/StrManipulateUtil.h"

namespace ETG::Testing
{
    //=================================================================================================================
    //  DirectionCoverage
    //=================================================================================================================

    void DirectionCoverage::Observe(const Direction direction)
    {
        const auto index = static_cast<size_t>(direction);
        if (index < DirectionCount) Seen[index] = true;
    }

    bool DirectionCoverage::HasSeen(const Direction direction) const
    {
        const auto index = static_cast<size_t>(direction);
        return index < DirectionCount && Seen[index];
    }

    size_t DirectionCoverage::SeenCount() const
    {
        size_t count = 0;
        for (const bool seen : Seen)
            if (seen)
                count++;

        return count;
    }

    std::string DirectionCoverage::Describe() const
    {
        std::string missing;
        for (size_t i = 0; i < DirectionCount; ++i)
        {
            if (Seen[i]) continue;

            if (!missing.empty()) missing += ", ";

            //The enum's own names, straight out of the BOOST_DESCRIBE_ENUM on Direction, so this text can never
            //drift away from the enum the way a hand-written name table would
            missing += EnumToString(static_cast<Direction>(i));
        }

        std::string text = std::to_string(SeenCount()) + " / " + std::to_string(DirectionCount) + " seen";
        if (!missing.empty()) text += " - missing: " + missing;

        return text;
    }

    //=================================================================================================================
    //  TravelProbe
    //=================================================================================================================

    void TravelProbe::Start(const GameObjectBase* target)
    {
        if (Tracking) return;                       //a measurement in flight is never restarted from under itself
        if (!GameClass::IsValid(target)) return;    //null, or already destroyed

        Target = target;
        StartPosition = target->GetPosition();
        LastPosition = StartPosition;
        PathLength = 0.f;
        Seconds = 0.f;
        Tracking = true;
        Started = true;
        TargetGone = false;
    }

    void TravelProbe::Tick()
    {
        if (!Tracking) return;

        //The object may have been destroyed since the last frame (a bullet that hit a wall, an enemy that died).
        //Freeze where we are and remember why, instead of reading a dangling pointer
        if (!GameClass::IsValid(Target))
        {
            Tracking = false;
            TargetGone = true;
            Target = nullptr;
            return;
        }

        const ETG::Vector2f position = Target->GetPosition();
        const ETG::Vector2f step = position - LastPosition;

        PathLength += std::sqrt(step.x * step.x + step.y * step.y);
        LastPosition = position;
        Seconds += Time::FrameTick;
    }

    void TravelProbe::Reset()
    {
        Target = nullptr;
        StartPosition = {};
        LastPosition = {};
        PathLength = 0.f;
        Seconds = 0.f;
        Tracking = false;
        Started = false;
        TargetGone = false;
    }

    float TravelProbe::GetDisplacement() const
    {
        const ETG::Vector2f delta = LastPosition - StartPosition;
        return std::sqrt(delta.x * delta.x + delta.y * delta.y);
    }

    float TravelProbe::GetAverageSpeed() const
    {
        return Seconds > 0.f ? GetDisplacement() / Seconds : 0.f;
    }

    //=================================================================================================================
    //  Stopwatch
    //=================================================================================================================

    void Stopwatch::Restart()
    {
        Seconds = 0.f;
        Running = true;
    }

    void Stopwatch::Reset()
    {
        Seconds = 0.f;
        Running = false;
    }

    void Stopwatch::Tick()
    {
        if (Running) Seconds += Time::FrameTick;
    }

    std::string Stopwatch::Describe(const float target) const
    {
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%.2f s / %.2f s", Seconds, target);
        return buffer;
    }
}
