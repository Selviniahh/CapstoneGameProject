#include "InteractiveTestRegistry.h"
#include <utility>

namespace ETG::Testing
{
    std::vector<InteractiveTestInfo>& InteractiveTestRegistry::All()
    {
        //Function-local static: the registrars run during static initialisation, before main, and this guarantees
        //the vector exists by the time the first of them asks for it no matter what order the linker picked
        static std::vector<InteractiveTestInfo> tests;
        return tests;
    }

    void InteractiveTestRegistry::Add(InteractiveTestInfo info)
    {
        All().push_back(std::move(info));
    }
}
