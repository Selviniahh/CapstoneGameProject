#include <imgui.h>
#include "EngineUI.h"
#include "../../Core/GameObjectBase.h"
#include "../../Managers/TypeRegistry.h"
#include "../../Animation/Animation.h"
#include "UIUtils.h"

using namespace UIUtils;

void ETG::ImGuiSetRelativeOrientation(GameObjectBase* obj)
{
    //NOTE: Position
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Position");
    ImGui::SameLine();
    if (ImGui::InputFloat2("##PosX", &obj->RelativePos.x, "%.3f"))
    {
        //Revert what I've done last time and then apply RelativePos again
        const sf::Vector2f delta = {obj->RelativePos.x - obj->PrevRelativePos.x, obj->RelativePos.y - obj->PrevRelativePos.y};
        obj->DrawProps.Position += delta;
        obj->PrevRelativePos = obj->RelativePos;
    }

    //NOTE: Scale
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Scale");
    ImGui::SameLine();
    if (ImGui::InputFloat2("##Scale", &obj->RelativeScale.x, "%.3f"))
    {
        const sf::Vector2f delta{obj->RelativeScale.x - obj->PrevRelativeScale.x, obj->RelativeScale.y - obj->PrevRelativeScale.y};
        obj->DrawProps.Scale += delta;
        obj->PrevRelativeScale = obj->RelativeScale;
    }

    //NOTE: Rotation
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Rotation");
    ImGui::SameLine();
    ImGui::SliderFloat("##Rot", &obj->RelativeRotation, 0.0f, 360.0f);
}

// When the UI changes, recalculate the final position from BasePosition.
void ETG::ImGuiSetAbsoluteOrientation(GameObjectBase* obj)
{
    //NOTE: Position
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Position");
    ImGui::SameLine();
    if (ImGui::InputFloat2("##PosX", &obj->Position.x, "%.3f"))
    {
    }

    //NOTE: Scale
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Scale");
    ImGui::SameLine();
    if (ImGui::InputFloat2("##Scale", &obj->Scale.x, "%.3f"))
    {
    }

    //NOTE: Rotation
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Rotation");
    ImGui::SameLine();
    ImGui::SliderFloat("##Rot", &obj->Rotation, 0.0f, 360.0f);
}

template <>
void ETG::ShowImGuiWidget<ETG::GameObjectBase*>(const char* label, GameObjectBase*& obj)
{
    if (!obj) return;

    if (ImGui::TreeNode(label))
    {
        TypeRegistry::ProcessObject(obj);
        ImGui::Spacing(); //Too much recursive visualization might get easily confusing. I wish to do all UI stuff as good as possible but I don't have time to learn in depth the ImGui library 
        ImGui::TreePop();
    }
}

template <>
void ETG::ShowImGuiWidget<bool>(const char* label, bool& value)
{
    BeginProperty(label);

    // Save current style
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 2.0f));

    //make checkbox a bit narrower
    ImGui::PushItemWidth(5.f);

    if (ImGui::Checkbox("##value", &value))
    {
        // Value changed
    }

    // Restore style
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();

    EndProperty();
}

template <>
void ETG::ShowImGuiWidget<std::string>(const char* label, std::string& value)
{
    BeginProperty(label);
    char buffer[256] = {};
    std::strncpy(buffer, value.c_str(), sizeof(buffer) - 1);
    if (ImGui::InputText("##value", buffer, sizeof(buffer)))
    {
        value = buffer;
    }
    EndProperty();
}

template <>
void ETG::ShowImGuiWidget<std::shared_ptr<sf::Texture>>(const char* label, std::shared_ptr<sf::Texture>& value)
{
    BeginProperty(label);

    DisplayTexture(value);

    EndProperty();
}

template <>
void ETG::ShowImGuiWidget<sf::Vector2<float>>(const char* label, sf::Vector2<float>& value)
{
    BeginProperty(label);
    ImGui::InputFloat2("##vector2", &value.x);
    EndProperty();
}

template <>
void ETG::ShowImGuiWidget<sf::Vector2<unsigned>>(const char* label, sf::Vector2u& value)
{
    BeginProperty(label);
        
    // Create temporary int values that ImGui can work with
    int tempValues[2] = { static_cast<int>(value.x), static_cast<int>(value.y) };
        
    // Use ImGui::InputInt2 to edit the values
    if (ImGui::InputInt2("##vector2u", tempValues))
    {
        // Validate and convert back to unsigned
        value.x = tempValues[0] >= 0 ? static_cast<unsigned>(tempValues[0]) : value.x;
        value.y = tempValues[1] >= 0 ? static_cast<unsigned>(tempValues[1]) : value.y;
    }
        
    EndProperty();
}

template <>
void ETG::ShowImGuiWidget<float>(const char* label, float& value)
{
    BeginProperty(label);
    ImGui::InputFloat("##value", &value);
    EndProperty();
}

template <>
void ETG::ShowImGuiWidget<int>(const char* label, int& value)
{
    BeginProperty(label);
    ImGui::InputInt("##value", &value);
    EndProperty();
}

template <>
void ETG::ShowImGuiWidget<Animation>(const char* label, Animation& value)
{
    DisplayAnimation(label, value);
}

template <>
void ETG::ShowImGuiWidget<sf::Rect<int>>(const char* label, sf::Rect<int>& value)
{
    // BeginProperty(label);
    if (ImGui::TreeNode(label))
    {
        DisplayIntRectangle(value);
        ImGui::TreePop();
    }
    // EndProperty();

}

template <>
void ETG::ShowImGuiWidget<AnimationManager>(const char* label, AnimationManager& value)
{
    DisplayAnimationManager(label, value);
}