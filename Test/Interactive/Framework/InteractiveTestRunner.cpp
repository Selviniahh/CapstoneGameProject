#include "InteractiveTestRunner.h"
#include <algorithm>
#include <exception>
#include <imgui.h>
#include <iostream>
#include "Engine/Managers/TypeRegistry.h"
#include "Engine/Platform/RenderWindow.h"
#include "InteractiveTestRegistry.h"

namespace ETG::Testing
{
    namespace
    {
        //The three status colours, in one place so the panel and the summary agree
        ImVec4 StatusColor(const CheckStatus status)
        {
            switch (status)
            {
            case CheckStatus::Passed: return {0.35f, 0.85f, 0.40f, 1.f};
            case CheckStatus::Failed: return {0.95f, 0.35f, 0.35f, 1.f};
            case CheckStatus::Pending: break;
            }
            return {0.85f, 0.75f, 0.30f, 1.f};
        }

        const char* StatusText(const CheckStatus status)
        {
            switch (status)
            {
            case CheckStatus::Passed: return "PASSED ";
            case CheckStatus::Failed: return "FAILED ";
            case CheckStatus::Pending: break;
            }
            return "PENDING";
        }
    }

    //A test that throws is shown in the panel AND written to stderr. The panel is for whoever is playing; the
    //stderr line is for whoever is reading a log afterwards, which is the only thing a build gate leaves behind
    void InteractiveTestRunner::ReportError(std::string message)
    {
        std::cerr << "[interactive test] " << message << std::endl;
        LastError = std::move(message);
    }

    InteractiveTestRunner::InteractiveTestRunner(GameManager& game) : Game(&game)
    {
        //The editor's hierarchy panel reflects an object through the type registry. Registering here rather than in
        //the game's RegisterGameTypes keeps the game side unaware that tests exist, and makes the runner behave
        //like any other object when it is clicked in the hierarchy
        TypeRegistry::RegisterType<InteractiveTestRunner>();
        REGISTER_BASE_CLASS(InteractiveTestRunner, GameObjectBase);

        IsGameObjectUISpecified = true;
        Env = std::make_unique<TestEnvironment>(game);

        //Start on the first registered test, so launching the app drops you straight into something playable
        if (!InteractiveTestRegistry::All().empty())
            RequestTest(0);
    }

    InteractiveTestRunner::~InteractiveTestRunner() = default;

    //=================================================================================================================
    //  Frame
    //=================================================================================================================

    void InteractiveTestRunner::Update()
    {
        GameObjectBase::Update();

        //Phase 2 of a switch: the previous world has been swept, so the queued test can safely build its own
        if (WaitingForSweep)
        {
            WaitingForSweep = false;
            LoadPendingTest();
        }

        if (Active)
        {
            Env->AdvanceClock();

            //A test that throws is a broken test, not a broken session: show it and stop ticking that test
            try
            {
                Active->Update(*Env);
            }
            catch (const std::exception& error)
            {
                ReportError(std::string("Update threw: ") + error.what());
                UnloadActiveTest();
            }

            RecordOutcome();
        }

        DrawPanel();
    }

    //=================================================================================================================
    //  Loading / unloading
    //=================================================================================================================

    void InteractiveTestRunner::RequestTest(const size_t index)
    {
        if (index >= InteractiveTestRegistry::All().size()) return;

        PendingIndex = index;

        //Tear the current world down NOW (this frame's sweep collects it) and build the new one next frame
        UnloadActiveTest();
        WaitingForSweep = true;
    }

    void InteractiveTestRunner::RestartActiveTest()
    {
        if (ActiveIndex != NoTest) RequestTest(ActiveIndex);
    }

    void InteractiveTestRunner::UnloadActiveTest()
    {
        if (Active)
        {
            RecordOutcome();

            //The test's chance to undo anything that lives outside its own spawned objects
            try
            {
                Active->TearDown(*Env);
            }
            catch (const std::exception& error)
            {
                ReportError(std::string("TearDown threw: ") + error.what());
            }
        }

        //Everything the test spawned goes, whether or not there was a test object left to ask
        Env->DestroyEverythingSpawned();
        Active.reset();
        ActiveIndex = NoTest;
    }

    void InteractiveTestRunner::LoadPendingTest()
    {
        if (PendingIndex == NoTest) return;

        const auto& tests = InteractiveTestRegistry::All();
        if (PendingIndex >= tests.size()) return;

        const InteractiveTestInfo& info = tests[PendingIndex];

        //A fresh environment per test: an empty spawn list and a clock starting at zero
        Env = std::make_unique<TestEnvironment>(*Game);

        Active = info.Create();
        Active->SetInfo(info.Name, info.Category);
        ActiveIndex = PendingIndex;
        PendingIndex = NoTest;
        LastError.clear();

        try
        {
            Active->SetUp(*Env);
        }
        catch (const std::exception& error)
        {
            ReportError("\"" + info.Name + "\" SetUp threw: " + error.what());

            //Recorded before the test object is dropped, so the summary reports a test that could not be built
            //rather than silently showing a test with no checks
            TestOutcome& outcome = Outcomes[info.Name];
            outcome.Visited = true;
            outcome.SetUpFailed = true;

            UnloadActiveTest();
            return;
        }

        Outcomes[Active->GetName()].Visited = true;
        RecordOutcome();
    }

    //=================================================================================================================
    //  Results
    //=================================================================================================================

    void InteractiveTestRunner::RecordOutcome() const
    {
        if (!Active) return;

        TestOutcome& outcome = Outcomes[Active->GetName()];
        outcome.Passed = Active->CountWithStatus(CheckStatus::Passed);
        outcome.Failed = Active->CountWithStatus(CheckStatus::Failed);
        outcome.Pending = Active->CountWithStatus(CheckStatus::Pending);
        outcome.Visited = true;
    }

    bool InteractiveTestRunner::AnyCheckFailed() const
    {
        for (const auto& [name, outcome] : Outcomes)
            if (outcome.Failed > 0 || outcome.SetUpFailed)
                return true;

        return false;
    }

    bool InteractiveTestRunner::AllTestsFullyResolved() const
    {
        for (const InteractiveTestInfo& info : InteractiveTestRegistry::All())
        {
            const auto it = Outcomes.find(info.Name);
            if (it == Outcomes.end() || !it->second.Visited) return false; //never even loaded
            if (it->second.SetUpFailed) return false;
            if (it->second.Pending > 0 || it->second.Failed > 0) return false;
        }

        return true;
    }

    void InteractiveTestRunner::PrintSummary() const
    {
        std::cout << "\n===== Interactive gameplay test summary =====\n";

        for (const InteractiveTestInfo& info : InteractiveTestRegistry::All())
        {
            const auto it = Outcomes.find(info.Name);
            if (it == Outcomes.end() || !it->second.Visited)
            {
                std::cout << "  [NOT RUN] " << info.Name << "\n";
                continue;
            }

            const TestOutcome& outcome = it->second;

            if (outcome.SetUpFailed)
            {
                std::cout << "  [BROKEN]  " << info.Name << "   its SetUp threw, the world was never built\n";
                continue;
            }

            const char* verdict = outcome.Failed > 0 ? "[FAILED] " : (outcome.Pending > 0 ? "[PENDING]" : "[PASSED] ");

            std::cout << "  " << verdict << " " << info.Name
                << "   passed " << outcome.Passed
                << ", failed " << outcome.Failed
                << ", pending " << outcome.Pending << "\n";
        }

        std::cout << "============================================\n";
    }

    //=================================================================================================================
    //  The panel
    //=================================================================================================================

    void InteractiveTestRunner::DrawPanel()
    {
        //Docked to the left of the logical canvas on first use only - the editor's own Details Pane owns the right
        //edge. Left movable/resizable on purpose: a test may want the screen
        ImGui::SetNextWindowPos(ImVec2(12.f, 12.f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(430.f, static_cast<float>(ETG::RenderWindow::LogicalSize.y) - 24.f), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Interactive Gameplay Tests"))
        {
            ImGui::End();
            return;
        }

        if (InteractiveTestRegistry::All().empty())
        {
            ImGui::TextWrapped("No interactive tests are registered. Add a .cpp under Test/Interactive/Tests "
                               "and list it in Test/CMakeLists.txt.");
            ImGui::End();
            return;
        }

        //<---------- The active test's header ---------->
        if (Active)
        {
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.f, 1.f), "%s", Active->GetName().c_str());
            ImGui::SameLine();
            ImGui::TextDisabled("(%s)", Active->GetCategory().c_str());

            if (const std::string instructions = Active->GetInstructions(); !instructions.empty())
                ImGui::TextWrapped("%s", instructions.c_str());
        }
        else
        {
            ImGui::TextDisabled("Loading...");
        }

        if (!LastError.empty())
            ImGui::TextColored(StatusColor(CheckStatus::Failed), "%s", LastError.c_str());

        ImGui::Separator();

        //<---------- Controls ---------->
        if (ImGui::Button("Restart test")) RestartActiveTest();
        ImGui::SameLine();

        //Reset only the verdicts, keeping the world as it is: useful after fixing your aim, not your code
        if (ImGui::Button("Reset checks") && Active) Active->ResetAllChecks();

        ImGui::SameLine();
        ImGui::TextDisabled("| world is rebuilt on restart");

        ImGui::Separator();

        DrawCheckList();

        //<---------- The test's own knobs ---------->
        if (Active)
        {
            ImGui::Separator();
            try
            {
                Active->DrawWidgets(*Env);
            }
            catch (const std::exception& error)
            {
                ReportError(std::string("DrawWidgets threw: ") + error.what());
            }
        }

        ImGui::Separator();
        DrawTestPicker();

        ImGui::End();
    }

    void InteractiveTestRunner::DrawCheckList() const
    {
        if (!Active) return;

        const std::vector<Check>& checks = Active->GetChecks();
        if (checks.empty())
        {
            ImGui::TextDisabled("This test declared no checks.");
            return;
        }

        ImGui::Text("Checks  %zu passed / %zu failed / %zu pending",
                    Active->CountWithStatus(CheckStatus::Passed),
                    Active->CountWithStatus(CheckStatus::Failed),
                    Active->CountWithStatus(CheckStatus::Pending));

        for (size_t i = 0; i < checks.size(); ++i)
        {
            const Check& check = checks[i];

            ImGui::PushID(static_cast<int>(i));

            ImGui::TextColored(StatusColor(check.Status), "[%s]", StatusText(check.Status));
            ImGui::SameLine();
            ImGui::TextWrapped("%s", check.Title.c_str());

            //The live line: progress while pending, the verdict's reason once resolved
            if (!check.Detail.empty())
            {
                ImGui::Indent();
                ImGui::TextWrapped("%s", check.Detail.c_str());
                ImGui::Unindent();
            }

            //What the player has to do. Only worth showing while there is still something to do
            if (!check.HowTo.empty() && check.Status == CheckStatus::Pending)
            {
                ImGui::Indent();
                ImGui::TextDisabled("-> %s", check.HowTo.c_str());
                ImGui::Unindent();
            }

            ImGui::PopID();
        }
    }

    void InteractiveTestRunner::DrawTestPicker()
    {
        if (!ImGui::CollapsingHeader("All tests", ImGuiTreeNodeFlags_DefaultOpen)) return;

        const auto& tests = InteractiveTestRegistry::All();

        //Grouped by category. The registry is not sorted, so the header is emitted whenever the category changes
        //against what was last drawn - which is why the list is walked category by category
        std::vector<std::string> categories;
        for (const InteractiveTestInfo& info : tests)
        {
            if (std::find(categories.begin(), categories.end(), info.Category) == categories.end())
                categories.push_back(info.Category);
        }

        for (const std::string& category : categories)
        {
            ImGui::TextDisabled("%s", category.c_str());

            for (size_t i = 0; i < tests.size(); ++i)
            {
                if (tests[i].Category != category) continue;

                const auto outcomeIt = Outcomes.find(tests[i].Name);
                const bool visited = outcomeIt != Outcomes.end() && outcomeIt->second.Visited;

                //Colour the entry by its last known verdict, so the list doubles as the session's report
                ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
                if (visited)
                {
                    if (outcomeIt->second.Failed > 0) color = StatusColor(CheckStatus::Failed);
                    else if (outcomeIt->second.Pending == 0) color = StatusColor(CheckStatus::Passed);
                    else color = StatusColor(CheckStatus::Pending);
                }

                ImGui::PushID(static_cast<int>(i));
                ImGui::PushStyleColor(ImGuiCol_Text, color);

                if (ImGui::Selectable(tests[i].Name.c_str(), i == ActiveIndex))
                    RequestTest(i);

                ImGui::PopStyleColor();
                ImGui::PopID();
            }
        }
    }
}
