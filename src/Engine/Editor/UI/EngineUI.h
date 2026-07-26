#pragma once
#include "../../Platform/Platform.h"
#include <imgui.h>
#include <iostream>
#include <memory>
#include <boost/type_index.hpp>
#include "../../Platform/Platform.h"

#include "UIUtils.h"
#include "../../../Utils/StrManipulateUtil.h"

class Animation;

namespace ETG
{
    class GameObjectBase;
    using namespace UIUtils;


    //Base template forwards to the appropriate implementation
    template <typename T>
    void ShowImGuiWidget(const char* label, T& value);

    //GameObjectBase*
    template <>
    void ShowImGuiWidget<GameObjectBase*>(const char* label, GameObjectBase*& obj);

    //Bool
    template <>
    void ShowImGuiWidget<bool>(const char* label, bool& value);

    //int
    template <>
    void ShowImGuiWidget<int>(const char* label, int& value);

    //String
    template <>
    void ShowImGuiWidget<std::string>(const char* label, std::string& value);

    //shared-ptr<ETG::Texture>
    template <>
    void ShowImGuiWidget<std::shared_ptr<ETG::Texture>>(const char* label, std::shared_ptr<ETG::Texture>& value);

    //vector2<float>
    template <>
    void ShowImGuiWidget<ETG::Vector2<float>>(const char* label, ETG::Vector2<float>& value);

    //sf::vector2<Vector2u>
    template <>
    void ShowImGuiWidget<ETG::Vector2u>(const char* label, ETG::Vector2u& value);

    //float
    template <>
    void ShowImGuiWidget<float>(const char* label, float& value);

    //Stat. NOTE: needed even though a Stat converts to float - the widget is picked by the member's declared type,
    //not by what it can convert to, so without this a described Stat would fall through to the "did you forget a
    //specialization" branch
    template <>
    void ShowImGuiWidget<StatModifier>(const char* label, StatModifier& value);

    //Animation
    template <>
    void ShowImGuiWidget<Animation>(const char* label, Animation& value);

    //ETG::Rect<int>
    template <>
    void ShowImGuiWidget<ETG::Rect<int>>(const char* label, ETG::Rect<int>& value);

    //AnimationManager
    template <>
    void ShowImGuiWidget<AnimationManager>(const char* label, AnimationManager& value);

    //Color
    template <>
    void ShowImGuiWidget<ETG::Color>(const char* label, ETG::Color& color);

    template <typename T>
    void ShowImGuiWidgetImpl(const char* label, T& value, std::false_type);

    //default implementation for enums
    template <typename T>
    void ShowImGuiWidgetImpl(const char* label, T& value, std::true_type);

    class EngineUI
    {
    public:
        //TODO: I need to do something with this two function later I done with reflection
        void ImGuiSetRelativeOrientation(GameObjectBase* obj);
        void ImGuiSetAbsoluteOrientation(GameObjectBase* obj);
    };

    //------------------------------IMPLEMENTATION----------------------------------------------------
    //This will firstly execute if in above none of declared templates satisfied.  
    template <typename T>
    void ShowImGuiWidget(const char* label, T& value)
    {
        ShowImGuiWidgetImpl(label, value, std::is_enum<T>{});
    }

    //Implementation for non-enum types. If non defined non enum exposed to UI, this template spceialization will be executed
    template <typename T>
    void ShowImGuiWidgetImpl(const char* label, T& value, std::false_type)
    {
        //If the value is child of GameObject. Try to downcast and try again. If not there's no other implementation given
        if constexpr (std::is_convertible_v<T, GameObjectBase*>)
        {
            if (auto* child = dynamic_cast<GameObjectBase*>(value))
            {
                ShowImGuiWidget<GameObjectBase*>(label, child);
                return;
            }
        }

        const std::string ErrorMessage = "Non enum Typename: " + boost::typeindex::type_id<T>().pretty_name() + " and variable name " + label + " not found. "
            "Did you define a specialized template in EngineUI.cpp for this typename?";
        std::cerr << ErrorMessage << std::endl;

        ImGui::Text(ErrorMessage.c_str());
    }

    //default implementation for enums
    template <typename T>
    void ShowImGuiWidgetImpl(const char* label, T& value, std::true_type)
    {
        BeginProperty(label);
        ImGui::Text(EnumToString(value));
        EndProperty();
    }
}
