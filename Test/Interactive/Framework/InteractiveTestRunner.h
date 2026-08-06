#pragma once
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include "Engine/Core/GameObjectBase.h"
#include "Engine/Core/SingleInstance.h"
#include "InteractiveTest.h"
#include "TestEnvironment.h"

//=====================================================================================================================
//  THE RUNNER - one world object that owns the active test, its environment, and the ImGui panel.
//
//  It is a GameObjectBase because that is the only thing GameManager ticks: being in the world list gets it an
//  Update() every frame, in the middle of the ImGui frame the editor opened, which is where its panel is built.
//
//  Only ONE test is loaded at a time, and that is deliberate: every test owns the whole world (its own hero, its own
//  enemies, its own tuning), so two of them running at once would be two tests fighting over one Hero::Get().
//  Switching tests tears the world down and builds the next one.
//
//  ---------------------------------------------------------------------------------------------------------------
//  The switch is spread over two frames, and it has to be:
//
//     frame N     MarkForDestroy on everything the old test spawned. GameManager sweeps them at the END of this
//                 frame's update, so they are still alive right now
//     frame N+1   the old world is really gone -> construct the new test and call its SetUp
//
//  Doing both in one frame would build the new hero while the old one was still alive, and every enemy that
//  captured Hero::Get() in its constructor would be holding the corpse.
//=====================================================================================================================

namespace ETG::Testing
{
    //What the panel shows in its per-test summary, and what the process exit code is computed from
    struct TestOutcome
    {
        size_t Passed{0};
        size_t Failed{0};
        size_t Pending{0};
        bool Visited{false}; //false until the test has been loaded at least once this session

        //Its SetUp threw, so its world was never built and its checks never even got declared. Counted as a
        //failure: a test that cannot be built is a broken test, and "0 failed checks" must not read as a pass
        bool SetUpFailed{false};
    };

    class InteractiveTestRunner : public GameObjectBase, public SingleInstance<InteractiveTestRunner>
    {
    public:
        explicit InteractiveTestRunner(GameManager& game);
        ~InteractiveTestRunner() override;

        //Drives the phase machine, ticks the active test, then builds the panel
        void Update() override;

        //The runner has no visual of its own - it is a panel, and the panel is ImGui's business
        void Draw() override
        {
        }

        //<---------- Driving from outside (the panel calls these too) ---------->

        //Queue a test to be loaded. Takes effect on the next frame (see the two-frame note above)
        void RequestTest(size_t index);

        //Rebuild the active test's world from scratch: same test, brand new hero, brand new checks
        void RestartActiveTest();

        //<---------- What the host asks when the window closes ---------->

        //True if any check in any test visited this session ended up Failed, or if a test could not be built at all
        [[nodiscard]] bool AnyCheckFailed() const;

        //True if every registered test was visited and left with no pending checks. This is what --strict turns
        //into an exit code, for "the build does not continue until somebody actually played through the tests"
        [[nodiscard]] bool AllTestsFullyResolved() const;

        //One line per test, printed to stdout when the session ends
        void PrintSummary() const;

    private:
        void DrawPanel();
        void DrawCheckList() const;
        void DrawTestPicker();

        //Destroys the active test's world and forgets the test object. The next frame sets the queued one up
        void UnloadActiveTest();

        //Constructs the queued test and runs its SetUp
        void LoadPendingTest();

        //Copies the active test's counts into Outcomes, so the summary survives switching away from it
        void RecordOutcome() const;

        //Shows a message in the panel and writes it to stderr, so a failure is visible both while playing and in
        //the log a build gate leaves behind
        void ReportError(std::string message);

        GameManager* Game{nullptr};

        //The world handed to the active test. Recreated per test, so its spawn list and its clock start clean
        std::unique_ptr<TestEnvironment> Env;
        std::unique_ptr<InteractiveTest> Active;

        //Index into InteractiveTestRegistry::All()
        size_t ActiveIndex{NoTest};
        size_t PendingIndex{NoTest};

        //Set when a world was torn down and the queued test may not be built until the next frame
        bool WaitingForSweep{false};

        //Keyed by test name so a test that moves in the list keeps its result
        mutable std::map<std::string, TestOutcome> Outcomes;

        //Errors thrown out of a test's SetUp / Update are caught and shown in the panel instead of taking the
        //whole session down: a broken test is the thing being tested
        std::string LastError;

        static constexpr size_t NoTest = static_cast<size_t>(-1);

        BOOST_DESCRIBE_CLASS(InteractiveTestRunner, (GameObjectBase), (), (), ())
    };
}
