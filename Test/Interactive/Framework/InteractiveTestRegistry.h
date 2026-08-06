#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "InteractiveTest.h"

//=====================================================================================================================
//  THE TEST LIST. A test announces itself with one line at the bottom of its .cpp:
//
//      ETG_INTERACTIVE_TEST(HeroDirectionTest, "Hero", "Is all directions being switched?");
//                           ^ your class      ^ group  ^ what the panel shows in the list
//
//  There is no central file listing the tests: the macro creates a namespace-scope object whose constructor files
//  the test into the registry before main() runs. Adding a test means adding a file (plus its line in
//  Test/CMakeLists.txt), never editing a list somebody else is also editing.
//
//  The category is only used to group the panel's list ("Hero", "Guns", "Enemies", "Items", ...). Invent one when
//  none of the existing ones fits.
//=====================================================================================================================

namespace ETG::Testing
{
    //One registered test: how it is shown, and how to build a fresh instance of it
    struct InteractiveTestInfo
    {
        std::string Name;
        std::string Category;

        //A FACTORY, not an instance. The runner builds the test when it is selected and destroys it when it is
        //left, so restarting a test genuinely starts from zero - the test object itself carries no stale state
        std::function<std::unique_ptr<InteractiveTest>()> Create;
    };

    class InteractiveTestRegistry
    {
    public:
        //Every registered test, in registration order (which is link order - do not rely on it for anything but
        //display; the panel sorts by category anyway)
        static std::vector<InteractiveTestInfo>& All();

        //Called by the macro below. No reason to call it by hand
        static void Add(InteractiveTestInfo info);
    };

    //The object the macro instantiates. Its only job is to run Add() during static initialisation
    struct InteractiveTestRegistrar
    {
        InteractiveTestRegistrar(std::string name, std::string category,
                                 std::function<std::unique_ptr<InteractiveTest>()> create)
        {
            InteractiveTestRegistry::Add({std::move(name), std::move(category), std::move(create)});
        }
    };
}

//NOTE: the registrar lives in an anonymous namespace so two test files can never collide on the symbol, and the
//test sources are compiled straight into the executable (not into a static library) precisely because a linker is
//free to drop an object file nobody references - which would silently drop the test from the list
#define ETG_INTERACTIVE_TEST(TestClass, Category, DisplayName)                                     \
    namespace                                                                                      \
    {                                                                                              \
        const ::ETG::Testing::InteractiveTestRegistrar TestClass##_Registrar{                      \
            DisplayName, Category,                                                                 \
            [] { return std::unique_ptr<::ETG::Testing::InteractiveTest>(new TestClass()); }};     \
    }
