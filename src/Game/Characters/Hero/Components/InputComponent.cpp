#include "InputComponent.h"
#include <imgui.h>

#include "HeroAnimComp.h"
#include "HeroMoveComp.h"
#include "../Hero.h"
#include "../../../Managers/GameManager.h"
#include "../../../../Engine/Editor/Engine.h"
#include "../../../../Engine/Managers/InputManager.h"
#include "../../../../Utils/DirectionUtils.h"
#include "../HeroDirections.h"
#include "../../../../Utils/Math.h"
#include "../../../../Utils/StrManipulateUtil.h"

//This class reads input and files requests on the Hero (RequestDash) or sets values the state machine's guards read
//(CurrentDir, IsShooting). It never assigns a state: HeroStateMachine decides, HeroAnimComp then draws it.

namespace ETG
{
    InputComponent::InputComponent()
    {
        IsGameObjectUISpecified = true;
        // SetObjectNameToSelfClassName();
    }

    void InputComponent::Update(Hero& hero) const
    {
        //Shooting input lives here (game side); only registered while the game window has focus,
        //matching the old InputManager behavior of freezing input when the editor UI captures the mouse.
        if (Engine::IsGameWindowFocused())
            hero.IsShooting = ETG::Mouse::isButtonPressed(ETG::Mouse::Left);

        UpdateDirection(hero);
        HandleGunSwitch(hero);
        HandleDash(hero);

        //Reload the gun if R pressed. NOTE: this used to check "not dashing", which also let a dead hero reload.
        //CanShoot covers the dash case plus the dead / hit cases the old check missed
        if (hero.CanShoot() && ETG::Keyboard::isKeyPressed(ETG::Keyboard::R))
        {
            hero.CurrentGun->Reload();
        }
    }

    void InputComponent::UpdateDirection(Hero& hero) const
    {
        // Hero-relative mouse angle in [0..360) is game logic, so it's computed here from the engine's world mouse
        // position instead of inside InputManager. The enemies measure their angle with the same helper
        const float angle = DirectionUtils::GetAngleToTarget(InputManager::WorldMousePos, hero.GetPosition());

        // Store on the Hero (used for gun rotation)
        hero.AimAngle = angle;

        //NOTE: If it's Dash Set Hero's Direction based on the Keyboard Key
        if (hero.GetState() == HeroStateEnum::Dash)
        {
            hero.CurrentDir = HeroDirections::GetDashFacing();
        }
        else //NOTE: If it's not Dash, set Hero's input based on Mouse Angle.
        {
            hero.CurrentDir = DirectionUtils::GetDirectionFromAngle(angle);
        }
    }

    //NOTE: Input files a request, it does not start a dash. Whether the hero is already dashing, dead or mid-hit is
    //not input's business to know: the state machine either has a legal transition into Dash right now or it doesn't
    void InputComponent::HandleDash(Hero& hero)
    {
        if (!ETG::Mouse::isButtonPressed(ETG::Mouse::Right)) return;

        //NOTE: a dash in flight owns its direction. GetDashEnum() records LastDashDirection as a side effect, and
        //that is what GetDashFacing() - and therefore the hero's facing and sprite flip - is resolved from. Calling
        //it while dashing let a key pressed mid-dash turn the hero and swap his dash animation under him. Holding
        //the button still chains dashes; the next one just reads the keys when it actually starts
        if (hero.GetState() == HeroStateEnum::Dash) return;

        // If you put breakpoint this line, direction enum will always be unknown so put breakpoint below this line to capture dash direction
        hero.RequestDash(HeroDirections::GetDashEnum());
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

        //Modify Direction ranges as table.
        //
        //NOTE: This edits DirectionUtils::GetRanges(), the one table the whole game reads. It used to edit a copy
        //owned by this component, while the enemies faced their target through a second, hand-written copy of the
        //same numbers inside DirectionUtils - so tuning the hero here silently left them behind.
        //The arcs are half open, [min, max), which is why a value can be typed on a boundary without landing in two
        //arcs at once the way it used to
        if (ImGui::TreeNode("Direction Ranges"))
        {
            if (ImGui::Button("Reset to default")) DirectionUtils::ResetRangesToDefault();

            auto& ranges = DirectionUtils::GetRanges();

            if (ImGui::BeginTable("split2", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
            {
                //       |unmodified  key|           old               new             value
                std::map<std::pair<float, float>, std::pair<std::pair<float, float>, Direction>> keysToUpdate;

                //First default two strings to indicate the categories
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Range [min, max)");

                ImGui::TableNextColumn();
                ImGui::Text("Direction");

                for (const auto& [key, value] : ranges)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    //Make a copy of the current key that we can modify
                    std::pair<float, float> editedKey = key;
                    bool changed = false;

                    ImGui::PushID(static_cast<int>(key.first * 10.f));
                    ImGui::PushID(static_cast<int>(key.second * 10.f));

                    // Set a fixed width for both input fields
                    const float availWidth = ImGui::GetContentRegionAvail().x;
                    const float spacing = ImGui::GetStyle().ItemSpacing.x;
                    const float inputWidth = (availWidth - spacing) / 2.0f;

                    // First input field
                    ImGui::SetNextItemWidth(inputWidth);
                    const bool valueChanged = ImGui::InputFloat("##Key", &editedKey.first, 0.f, 0.f, "%.1f");
                    const bool lostFocusAfterEdit = ImGui::IsItemDeactivatedAfterEdit();

                    //If range is not between 0,360, revert
                    editedKey.first = std::max(0.f, std::min(360.f, editedKey.first));

                    // Second input field
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(inputWidth);
                    const bool valueChanged2 = ImGui::InputFloat("##Key2", &editedKey.second, 0.f, 0.f, "%.1f");
                    const bool lostFocusAfterEdit2 = ImGui::IsItemDeactivatedAfterEdit();

                    //If range is not between 0,360, revert
                    editedKey.second = std::max(0.f, std::min(360.f, editedKey.second));

                    if ((valueChanged && lostFocusAfterEdit) || (valueChanged2 && lostFocusAfterEdit2))
                        changed = true;

                    if (changed)
                        keysToUpdate[key] = std::pair<std::pair<float, float>, Direction>{editedKey, value};

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", EnumToString(value));

                    ImGui::PopID();
                    ImGui::PopID();
                }

                //If there are any change, remove from the table and add it back again
                for (const auto& [oldKey, newKeyAndValue] : keysToUpdate)
                {
                    //remove old entry
                    ranges.erase(oldKey);

                    //Add new entry
                    ranges[newKeyAndValue.first] = newKeyAndValue.second;
                }
            }
            ImGui::EndTable();

            ImGui::TreePop();
        }
    }
}
