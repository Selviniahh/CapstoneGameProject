#include "InputComponent.h"
#include <imgui.h>
#include "../Hero.h"
#include "../../Managers/InputManager.h"
#include "../../Utils/DirectionUtils.h"
#include "../../Utils/Math.h"
#include "../../Utils/StrManipulateUtil.h"

namespace ETG
{
    InputComponent::InputComponent()
    {
        IsGameObjectUISpecified = true;
        DirectionUtils::PopulateDirectionRanges(DirectionMap);
        // SetObjectNameToSelfClassName();
    }

    void InputComponent::Update(Hero& hero) const
    {
        UpdateDirection(hero);
    }


    void InputComponent::UpdateDirection(Hero& hero) const
    {
        // Convert mouse angle to [0..360)
        float angle = Math::RadiansToDegrees(InputManager::GetMouseAngleRelativeToHero());
        if (angle < 0.f) angle += 360.f;

        // Store on the Hero (or handle in input component)
        hero.MouseAngle = angle;
        hero.CurrentDirection = DirectionUtils::GetHeroDirectionFromAngle(DirectionMap, angle);
    }

    HeroDashEnum InputComponent::GetDashDirectionEnum()
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && sf::Keyboard::isKeyPressed(sf::Keyboard::W)) return HeroDashEnum::Dash_BackWard;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && sf::Keyboard::isKeyPressed(sf::Keyboard::W)) return HeroDashEnum::Dash_BackWard;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && sf::Keyboard::isKeyPressed(sf::Keyboard::S)) return HeroDashEnum::Dash_BackWard;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && sf::Keyboard::isKeyPressed(sf::Keyboard::S)) return HeroDashEnum::Dash_BackWard;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) return HeroDashEnum::Dash_Left;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) return HeroDashEnum::Dash_Right;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) return HeroDashEnum::Dash_Back;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) return HeroDashEnum::Dash_Front;
        return HeroDashEnum::Unknown;
    }

    void InputComponent::PopulateSpecificWidgets()
    {
        ComponentBase::PopulateSpecificWidgets();
        IsGameObjectUISpecified = true;

        //Modify Direction map as table 
        if (ImGui::TreeNode("DirectionMap"))
        {
            if (ImGui::BeginTable("split2", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
            {
                //       |unmodified  key|           old           new         value
                std::map<std::pair<int, int>, std::pair<std::pair<int, int>, Direction>> keysToUpdate;

                //First default two strings to indicate the categories
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Range");

                ImGui::TableNextColumn();
                ImGui::Text("Direction");

                for (const auto& [key, value] : DirectionMap)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    //Make a copy of the current key that we can modify
                    std::pair<int, int> editedKey = key;
                    bool changed = false;

                    ImGui::PushID(key.first);
                    ImGui::PushID(key.second);

                    // Set a fixed width for both input fields
                    const float availWidth = ImGui::GetContentRegionAvail().x;
                    const float spacing = ImGui::GetStyle().ItemSpacing.x;
                    const float inputWidth = (availWidth - spacing) / 2.0f;

                    // First input field
                    ImGui::SetNextItemWidth(inputWidth);
                    const bool valueChanged = ImGui::InputInt("##Key", &editedKey.first, 0, 0);
                    const bool lostFocusAfterEdit = ImGui::IsItemDeactivatedAfterEdit();

                    //If range is not between 0,360, revert
                    if (!Math::IsInRange(editedKey.first, 0, 360))
                    {
                        editedKey.first = std::max(0, std::min(360, editedKey.first));
                    }

                    // Second input field
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(inputWidth);
                    const bool valueChanged2 = ImGui::InputInt("##Key2", &editedKey.second, 0, 0);
                    const bool lostFocusAfterEdit2 = ImGui::IsItemDeactivatedAfterEdit();

                    //If range is not between 0,360, revert
                    if (!Math::IsInRange(editedKey.second, 0, 360))
                    {
                        editedKey.second = std::max(0, std::min(360, editedKey.second));
                    }

                    if ((valueChanged && lostFocusAfterEdit) || (valueChanged2 && lostFocusAfterEdit2))
                        changed = true;

                    if (changed)
                        keysToUpdate[key] = std::pair<std::pair<int, int>, Direction>{editedKey, value};

                    ImGui::TableNextColumn();
                    ImGui::Text(EnumToString(value));

                    ImGui::PopID();
                    ImGui::PopID();
                }

                //If there are any change, remove from DirectionMap and add it back again 
                for (const auto& [oldKey, newKeyAndValue] : keysToUpdate)
                {
                    //remove old entry
                    DirectionMap.erase(oldKey);

                    //Add new entry
                    DirectionMap[newKeyAndValue.first] = newKeyAndValue.second;
                }
            }
            ImGui::EndTable();

            ImGui::TreePop();
        }
    }
}
