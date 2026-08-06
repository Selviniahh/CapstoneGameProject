#include "InteractiveTest.h"
#include <cmath>
#include <cstdio>
#include <utility>

namespace ETG::Testing
{
    //=================================================================================================================
    //  CheckHandle - every method tolerates an invalid handle, so a test that forgot to assign one in SetUp
    //  misbehaves by showing nothing rather than by crashing mid-session
    //=================================================================================================================

    CheckStatus CheckHandle::GetStatus() const
    {
        const Check* check = Owner ? Owner->GetCheck(Index) : nullptr;
        return check ? check->Status : CheckStatus::Pending;
    }

    void CheckHandle::Pass(std::string detail) const
    {
        Check* check = Owner ? Owner->GetCheck(Index) : nullptr;
        if (!check || check->Status != CheckStatus::Pending) return; //first verdict wins

        check->Status = CheckStatus::Passed;
        if (!detail.empty()) check->Detail = std::move(detail);
    }

    void CheckHandle::Fail(std::string reason) const
    {
        Check* check = Owner ? Owner->GetCheck(Index) : nullptr;
        if (!check || check->Status != CheckStatus::Pending) return;

        check->Status = CheckStatus::Failed;
        check->Detail = std::move(reason);
    }

    void CheckHandle::PassIf(const bool condition, std::string detail) const
    {
        if (condition) Pass(std::move(detail));
    }

    void CheckHandle::FailIf(const bool condition, std::string reason) const
    {
        if (condition) Fail(std::move(reason));
    }

    bool CheckHandle::ExpectNear(const float actual, const float expected, const float tolerance, const char* unit) const
    {
        const bool passed = std::fabs(actual - expected) <= tolerance;

        //The numbers go into the detail text either way: a passing measurement is worth reading too, because a
        //value that only just scraped inside the tolerance is the interesting kind of pass
        char buffer[192];
        std::snprintf(buffer, sizeof(buffer), "measured %.2f%s, expected %.2f%s (+-%.2f)",
                      actual, unit, expected, unit, tolerance);

        if (passed) Pass(buffer);
        else Fail(buffer);

        return passed;
    }

    void CheckHandle::Progress(std::string text) const
    {
        Check* check = Owner ? Owner->GetCheck(Index) : nullptr;
        if (!check || check->Status != CheckStatus::Pending) return; //never overwrite a verdict's reason

        check->Detail = std::move(text);
    }

    void CheckHandle::Reset() const
    {
        if (Check* check = Owner ? Owner->GetCheck(Index) : nullptr)
        {
            check->Status = CheckStatus::Pending;
            check->Detail.clear();
        }
    }

    //=================================================================================================================
    //  InteractiveTest
    //=================================================================================================================

    CheckHandle InteractiveTest::AddCheck(std::string title, std::string howTo)
    {
        Checks.push_back(Check{std::move(title), std::move(howTo), {}, CheckStatus::Pending});
        return CheckHandle{this, Checks.size() - 1};
    }

    Check* InteractiveTest::GetCheck(const size_t index)
    {
        return index < Checks.size() ? &Checks[index] : nullptr;
    }

    const Check* InteractiveTest::GetCheck(const size_t index) const
    {
        return index < Checks.size() ? &Checks[index] : nullptr;
    }

    void InteractiveTest::SetInfo(std::string name, std::string category)
    {
        Name = std::move(name);
        Category = std::move(category);
    }

    void InteractiveTest::ResetAllChecks()
    {
        for (Check& check : Checks)
        {
            check.Status = CheckStatus::Pending;
            check.Detail.clear();
        }
    }

    size_t InteractiveTest::CountWithStatus(const CheckStatus status) const
    {
        size_t count = 0;
        for (const Check& check : Checks)
            if (check.Status == status)
                count++;

        return count;
    }
}
