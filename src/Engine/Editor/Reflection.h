#pragma once
#include <boost/describe.hpp>
#include <boost/type_index.hpp>
#include "../Platform/Platform.h"
#include <imgui.h>
#include "UI/EngineUI.h"
#include "../../Utils/StrManipulateUtil.h"

class Reflection
{
public:
    //does type has any members
    template <class T,
              class Md = boost::describe::describe_members<T, boost::describe::mod_any_access>>
    static constexpr bool HasMembers()
    {
        return boost::mp11::mp_size<Md>::value > 0;
    }

    //Bases:
    template <class T,
              class Bd = boost::describe::describe_bases<T, boost::describe::mod_any_access>,
              class Md = boost::describe::describe_members<T, boost::describe::mod_any_access>>
    static void PopulateReflection(const T& t)
    {
        // Process base classes first
        boost::mp11::mp_for_each<Bd>([&]<typename T0>(T0)
        {
            using BTN = typename T0::type;

            //If the current base has any members, create treenode
            if constexpr (HasMembers<BTN>())
            {
                //Set label name. You can skip reading this part.
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                std::string baseLabel = "Base: " + ETG::TypeNameToString<BTN>();

                if (ImGui::TreeNode(baseLabel.c_str()))
                {
                    PopulateReflection((BTN const&)t);
                    ImGui::TreePop();
                }
            }
            //If the object doesn't have any members, create it without tree node. 
            else
            {
                PopulateReflection((BTN const&)t);
            }
        });

        // Process members of the current class
        if constexpr (boost::mp11::mp_size<Md>::value > 0)
        {
            // For now disable tree drawing
            const bool TreeRequired = false; //boost::mp11::mp_size<Md>::value > 5
            const std::string ObjectLabel = "Object: " + ETG::TypeNameToString<T>();

            // If tree is required, create it and check if open
            bool shouldProcessMembers = true;
            if (TreeRequired) shouldProcessMembers = ImGui::TreeNode(ObjectLabel.c_str());

            // Only process members if we should (either no tree needed or tree is open)
            if (shouldProcessMembers)
            {
                boost::mp11::mp_for_each<Md>([&](auto D)
                {
                    using member_type = std::decay_t<decltype(t.*D.pointer)>;
                    auto VariableName = D.name;
                    auto* ValuePtr = &(t.*D.pointer);
                    member_type& valueRef = const_cast<member_type&>(*(ValuePtr));

                    ETG::ShowImGuiWidget<member_type>(VariableName, valueRef);
                });

                // Only close the tree if one was opened
                if (TreeRequired) ImGui::TreePop();
            }
        }
    }
};

namespace ETG
{
    // Declared in EngineUI.h, defined here: the fallback widget for a described plain struct is just this walk.
    // A gun can therefore group its tunables into small named structs and every field still reaches the panel.
    template <typename T>
    void PopulateDescribedStruct(const char* label, T& value)
    {
        if (ImGui::TreeNode(label))
        {
            Reflection::PopulateReflection<T>(value);
            ImGui::TreePop();
        }
    }
}
