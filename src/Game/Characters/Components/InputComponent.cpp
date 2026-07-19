#include "InputComponent.h"
#include <imgui.h>

#include "HeroAnimComp.h"
#include "HeroMoveComp.h"
#include "../Hero.h"
#include "../../Managers/GameManager.h"
#include "../../../Engine/Managers/InputManager.h"
#include "../../../Utils/DirectionUtils.h"
#include "../../../Utils/Math.h"
#include "../../../Utils/StrManipulateUtil.h"

//This class will set `HeroPtr->CurrentHeroState`
//HeroAnimComp will set real animation based on HeroPtr->CurrentHeroState

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
        HandleGunSwitch(hero);
        HandleDash(hero);

        //Reload the gun if R pressed and hero is not dashing 
        if (!hero.AnimationComp->IsDashing && ETG::Keyboard::isKeyPressed(ETG::Keyboard::R))
        {
            hero.CurrentGun->Reload();
        }
    }

    void InputComponent::UpdateDirection(Hero& hero) const
    {
        // Convert mouse angle to [0..360)
        float angle = Math::RadiansToDegrees(InputManager::GetMouseAngleRelativeToHero());
        if (angle < 0.f) angle += 360.f;

        // Store on the Hero (used for gun rotation)
        hero.MouseAngle = angle;

        //NOTE: If it's Dash Set Hero's Direction based on the Keyboard Key
        if (hero.GetState() == HeroStateEnum::Dash)
        {
            hero.CurrentDirection = DirectionUtils::GetDirectionFromDash();
        }
        else //NOTE: If it's not Dash, set Hero's input based on Mouse Angle. 
        {
            hero.CurrentDirection = DirectionUtils::GetHeroDirectionFromAngle(DirectionMap, angle);
        }
    }

    void InputComponent::HandleDash(const Hero& hero)
    {
        // If right-clicked, start dashing if possible
        if (ETG::Mouse::isButtonPressed(ETG::Mouse::Right) && !hero.AnimationComp->IsDashing && hero.MoveComp->IsDashAvailable())
        {
            const HeroDashEnum dashDirection = DirectionUtils::GetDashDirectionEnum(); // If you put breakpoint this line, direction enum will always be unknown so put breakpoint below this line to capture dash direction
            if (dashDirection != HeroDashEnum::Unknown)
            {
                hero.AnimationComp->StartDash(dashDirection);
            }
        }
    }

    void InputComponent::HandleGunSwitch(Hero& hero) const
    {
        //If any imgui window hovered, return
        const bool anyWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
        if (anyWindowHovered) return;

        // If gun is not reloading, not dead or hit or dash and mouse wheel scrolled, switch gun
        // The per-frame wheel delta is accumulated by GameManager::ProcessEvents
        if (hero.CanSwitchGuns() && InputManager::MouseWheelDelta != 0.f)
        {
            if (InputManager::MouseWheelDelta > 0)
                hero.SwitchToPreviousGun();

            else if (InputManager::MouseWheelDelta < 0)
                hero.SwitchToNextGun();

            gunSwitchHandled = true; // Set flag to indicate the event has been handled
            InputManager::MouseWheelDelta = 0.f; // Clear the delta to prevent it from being processed again
        }
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
