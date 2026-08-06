#pragma once
#include <array>
#include <string>
#include "Engine/Core/Direction.h"
#include "Engine/Platform/Platform.h"

//=====================================================================================================================
//  WATCHERS - the small observers gameplay tests keep needing. Each one is a plain member of your test that you feed
//  every frame from Update(); none of them knows anything about checks, so you decide what a result means.
//
//  They exist so that the hundredth test does not re-implement "has this been seen yet" and "how far did that go" for
//  the hundredth time. When you find yourself writing the same bookkeeping in a second test, it belongs here.
//=====================================================================================================================

namespace ETG
{
    class GameObjectBase;

    namespace Testing
    {
        //-------------------------------------------------------------------------------------------------------------
        //  Ticks off the eight facings as they are seen. The "did the player turn all the way around?" bookkeeping.
        //
        //      DirectionCoverage Coverage;             //member of the test
        //      Coverage.Observe(hero->CurrentDir);     //every frame in Update
        //      Check.Progress(Coverage.Describe());
        //      Check.PassIf(Coverage.IsComplete());
        //-------------------------------------------------------------------------------------------------------------
        class DirectionCoverage
        {
        public:
            //Records one facing. Calling it with the same direction for a hundred frames costs nothing
            void Observe(Direction direction);

            [[nodiscard]] bool HasSeen(Direction direction) const;
            [[nodiscard]] bool IsComplete() const { return SeenCount() == DirectionCount; }
            [[nodiscard]] size_t SeenCount() const;

            //Panel-ready text: "3 / 8 seen - missing: Down, DownLeft, Left, UpLeft, Up"
            [[nodiscard]] std::string Describe() const;

            void Reset() { Seen = {}; }

            static constexpr size_t DirectionCount = 8;

        private:
            std::array<bool, DirectionCount> Seen{};
        };

        //-------------------------------------------------------------------------------------------------------------
        //  Follows one object and measures how far it got, and how long that took. THE tool for "did it really move
        //  at the speed it claims" tests - the bullet-speed test is written entirely out of this.
        //
        //      Probe.Start(bullet);                    //when the thing you want to measure appears
        //      Probe.Tick();                           //every frame after that
        //      if (Probe.GetSeconds() >= 3.f)
        //          Check.ExpectNear(Probe.GetDisplacement(), expectedSpeed * 3.f, tolerance, " px");
        //
        //  The probe survives its target being destroyed mid-measurement (a bullet that hit something, an enemy that
        //  died): it freezes at the last values it saw and reports TargetDiedEarly(), so a test can tell "it stopped
        //  early" apart from "it travelled the wrong distance".
        //-------------------------------------------------------------------------------------------------------------
        class TravelProbe
        {
        public:
            //Begins a measurement. Passing null, or calling it while another measurement is running, does nothing -
            //so `if (!Probe.IsTracking()) Probe.Start(env.FindFirst<ProjectileBase>());` is safe every frame
            void Start(const GameObjectBase* target);

            //Advances the measurement by one frame. Call it every frame from Update()
            void Tick();

            //Stops measuring but keeps the numbers readable
            void Stop() { Tracking = false; }

            void Reset();

            [[nodiscard]] bool IsTracking() const { return Tracking; }
            [[nodiscard]] bool HasStarted() const { return Started; }
            [[nodiscard]] bool TargetDiedEarly() const { return TargetGone; }

            //Straight line from where the measurement started to where the object is now. This is the one to compare
            //against speed * time for anything travelling in a straight line, like a bullet
            [[nodiscard]] float GetDisplacement() const;

            //Total length of the path walked, summed per frame. Differs from the displacement for anything that
            //turns - a chasing enemy, a strafing player
            [[nodiscard]] float GetPathLength() const { return PathLength; }

            //Seconds spent measuring
            [[nodiscard]] float GetSeconds() const { return Seconds; }

            //Displacement divided by time, i.e. the speed the object actually managed. Zero before the first frame
            [[nodiscard]] float GetAverageSpeed() const;

            [[nodiscard]] const ETG::Vector2f& GetStartPosition() const { return StartPosition; }
            [[nodiscard]] const ETG::Vector2f& GetLastPosition() const { return LastPosition; }

        private:
            const GameObjectBase* Target{nullptr};
            ETG::Vector2f StartPosition{};
            ETG::Vector2f LastPosition{};
            float PathLength{0.f};
            float Seconds{0.f};
            bool Tracking{false};
            bool Started{false};
            bool TargetGone{false};
        };

        //-------------------------------------------------------------------------------------------------------------
        //  A seconds counter driven by the game's own clock, so it stops while the window is unfocused (the game is
        //  paused then, and a test measuring wall-clock time through a pause measures nothing).
        //-------------------------------------------------------------------------------------------------------------
        class Stopwatch
        {
        public:
            void Start() { Running = true; }
            void Stop() { Running = false; }
            void Restart();
            void Reset();

            //Call every frame; only accumulates while running
            void Tick();

            [[nodiscard]] float GetSeconds() const { return Seconds; }
            [[nodiscard]] bool IsRunning() const { return Running; }
            [[nodiscard]] bool HasElapsed(const float seconds) const { return Seconds >= seconds; }

            //"1.42 s / 3.00 s" - panel-ready progress text for a timed check
            [[nodiscard]] std::string Describe(float target) const;

        private:
            float Seconds{0.f};
            bool Running{false};
        };
    }
}
