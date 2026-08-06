#pragma once
#include <cstddef>
#include <string>
#include <vector>

//=====================================================================================================================
//  INTERACTIVE GAMEPLAY TEST - the base class every test derives from.
//
//  An interactive test is a gameplay assertion that a HUMAN resolves by playing. It is displayed in the
//  "Interactive Gameplay Tests" ImGui panel as a line of text with a status:
//
//      [PENDING]  Is all directions being switched?      3 / 8 directions seen
//      [PASSED]   Is all directions being switched?      all 8 directions seen
//      [FAILED]   Bullet travels ShotSpeed px/s          measured 412 px, expected 500 px (+-25)
//
//  A test starts out PENDING and stays that way until your own Update() decides otherwise. Nothing times out,
//  nothing polls: you look at the world every frame and call Pass() / Fail() on the check when you know.
//
//  --------------------------------------------------------------------------------------------------------------
//  WRITING A NEW TEST - the whole recipe
//  --------------------------------------------------------------------------------------------------------------
//
//      // Test/Interactive/Tests/MyThingTest.cpp
//      #include "../Framework/InteractiveTest.h"
//      #include "../Framework/TestEnvironment.h"
//      #include "../Framework/InteractiveTestRegistry.h"
//
//      using namespace ETG;
//      using namespace ETG::Testing;
//
//      class MyThingTest final : public InteractiveTest
//      {
//          CheckHandle DoesTheThing;   // one member per assertion
//
//          //1. Build the world this test needs. Every test gets an EMPTY world and fills it itself
//          void SetUp(TestEnvironment& env) override
//          {
//              Hero* hero = env.SpawnHero({0.f, 0.f});     //spawn the character...
//              hero->GetMoveComp()->MaxSpeed = 400.f;      //...and change whatever this test is about
//              env.SpawnEnemy<BulletMan>({80.f, 0.f});     //spawn whatever it has to interact with
//
//              //2. Declare the assertions. They show up as PENDING lines in the panel
//              DoesTheThing = AddCheck("Does the thing happen?", "Walk into the enemy");
//          }
//
//          //3. Called once per frame while this test is the active one. Resolve the checks here
//          void Update(TestEnvironment& env) override
//          {
//              DoesTheThing.PassIf(env.GetHero()->GetPosition().x > 50.f);
//          }
//      };
//
//      //4. One line to make the test appear in the panel's list
//      ETG_INTERACTIVE_TEST(MyThingTest, "Hero", "My thing works");
//
//  Then add the .cpp to ETG_INTERACTIVE_TEST_SOURCES in Test/CMakeLists.txt and you are done.
//
//  --------------------------------------------------------------------------------------------------------------
//  WHAT THE FRAMEWORK GUARANTEES
//  --------------------------------------------------------------------------------------------------------------
//   * Every test gets its own world. Whatever you spawned through the TestEnvironment is destroyed when the test
//     is left or restarted, so tests never inherit each other's leftovers.
//   * SetUp runs one frame AFTER the previous test's objects were swept, so a previous Hero is truly gone before
//     yours is constructed (enemies capture Hero::Get() in their constructor - that pointer has to be yours).
//   * Update runs once per frame, at the top of the frame: the runner is the first object in the world list, so
//     what you read is the world as it stood at the end of the previous frame. That lag is one frame and it is
//     the same every frame, so anything measured over time (distance, duration, averages) is unaffected by it.
//=====================================================================================================================

namespace ETG::Testing
{
    class TestEnvironment;
    class InteractiveTest;

    //Where a single assertion currently stands. Everything starts Pending; the test decides the rest
    enum class CheckStatus
    {
        Pending, //nobody has decided yet - the player has not done the thing (or not done it wrong)
        Passed,  //the mechanic behaved
        Failed   //the mechanic misbehaved. A failed check never goes back to pending on its own
    };

    //One line in the panel
    struct Check
    {
        std::string Title;   //the assertion, e.g. "Is all directions being switched?"
        std::string HowTo;   //what the player has to do, e.g. "Sweep the mouse in a full circle"
        std::string Detail;  //live text: progress while pending, the reason once resolved
        CheckStatus Status{CheckStatus::Pending};
    };

    //A stable reference to one Check. Handed out by InteractiveTest::AddCheck and stored as a member of the test.
    //
    //NOTE: it deliberately holds an index rather than a Check*, so that adding more checks later (which can
    //reallocate the vector holding them) never invalidates the handles a test is already holding
    class CheckHandle
    {
    public:
        //A default-constructed handle points at nothing and every call on it is a silent no-op. That is what makes
        //`CheckHandle MyCheck;` safe to declare as a member and only assign inside SetUp
        CheckHandle() = default;
        CheckHandle(InteractiveTest* owner, size_t index) : Owner(owner), Index(index) {}

        //<---------- Resolving ---------->
        //All of these ignore a check that is already resolved: the FIRST verdict sticks. A mechanic that worked
        //once and then broke should be a Fail, so use FailIf for the broken condition and PassIf for the good one -
        //whichever fires first wins, which is exactly the behaviour you want out of a bug hunt

        void Pass(std::string detail = {}) const;      //verdict: the mechanic worked
        void Fail(std::string reason) const;           //verdict: it did not, and this is why

        //Pass the moment `condition` is true. Ignored while it is false, so you can call it every frame
        void PassIf(bool condition, std::string detail = {}) const;

        //Fail the moment `condition` is true. The counterpart of PassIf, for the "this must never happen" half
        void FailIf(bool condition, std::string reason) const;

        //A measurement check in one call: passes when `actual` is within `tolerance` of `expected`, fails otherwise,
        //and either way writes the three numbers into the detail text so the panel shows what was measured.
        //Returns whether it passed, for the rare caller that wants to branch on it
        bool ExpectNear(float actual, float expected, float tolerance, const char* unit = "") const;

        //<---------- Live text ---------->
        //Shown next to a pending check. Call it every frame with whatever counts as progress ("3 / 8 directions",
        //"1.4 s of 3.0 s"). Ignored once the check is resolved, so the final detail text is never overwritten
        void Progress(std::string text) const;

        //Puts the check back to Pending and clears its detail. The panel's "Reset checks" button does this to all
        //of them; a test can do it to one check on its own (a check that re-arms after each attempt)
        void Reset() const;

        //<---------- State ---------->
        [[nodiscard]] bool IsValid() const { return Owner != nullptr; }
        [[nodiscard]] bool IsPending() const { return GetStatus() == CheckStatus::Pending; }
        [[nodiscard]] bool IsPassed() const { return GetStatus() == CheckStatus::Passed; }
        [[nodiscard]] bool IsFailed() const { return GetStatus() == CheckStatus::Failed; }
        [[nodiscard]] CheckStatus GetStatus() const;

    private:
        InteractiveTest* Owner{nullptr};
        size_t Index{0};
    };

    //=================================================================================================================
    //  The base class. Derive, override SetUp, override Update, register with ETG_INTERACTIVE_TEST
    //=================================================================================================================
    class InteractiveTest
    {
    public:
        virtual ~InteractiveTest() = default;

        //<---------- The four hooks ---------->

        //Build this test's world and declare its checks. Called once, on an EMPTY world (the previous test's
        //objects have already been destroyed and swept). Spawn everything through `env` so it gets cleaned up
        virtual void SetUp(TestEnvironment& env) = 0;

        //Called once per frame while this test is active, after the world has updated. This is where checks are
        //resolved. Do not spawn one-off objects here without going through `env`, or they will outlive the test
        virtual void Update(TestEnvironment& env)
        {
        }

        //Extra ImGui widgets for this test, drawn under its check list. Use it for the knobs a test wants exposed
        //(a speed slider, a "spawn another enemy" button). Optional - most tests need nothing here
        virtual void DrawWidgets(TestEnvironment& env)
        {
        }

        //Called right before the test's objects are destroyed. Only needed to undo something that lives OUTSIDE
        //the spawned world - a global you changed, a stat modifier you put on a shared object. Spawned objects
        //are cleaned up for you
        virtual void TearDown(TestEnvironment& env)
        {
        }

        //One or two sentences shown at the top of the panel telling the player what to do. Optional
        [[nodiscard]] virtual std::string GetInstructions() const { return {}; }

        //<---------- Used by the runner and the panel ---------->
        [[nodiscard]] const std::vector<Check>& GetChecks() const { return Checks; }
        [[nodiscard]] const std::string& GetName() const { return Name; }
        [[nodiscard]] const std::string& GetCategory() const { return Category; }

        //Set by the runner right after construction, from what the registration macro was given
        void SetInfo(std::string name, std::string category);

        //Every check back to Pending. Wired to the panel's "Reset checks" button
        void ResetAllChecks();

        [[nodiscard]] size_t CountWithStatus(CheckStatus status) const;

    protected:
        //Declare an assertion. Call it from SetUp and keep the returned handle as a member of your test.
        //  title: the question the check answers, in the panel's list
        //  howTo: what the player has to do for it to resolve. Shown greyed out under the title
        CheckHandle AddCheck(std::string title, std::string howTo = {});

    private:
        //CheckHandle reaches into these two
        friend class CheckHandle;
        [[nodiscard]] Check* GetCheck(size_t index);
        [[nodiscard]] const Check* GetCheck(size_t index) const;

        std::vector<Check> Checks;
        std::string Name{"Unnamed test"};
        std::string Category{"General"};
    };
}
