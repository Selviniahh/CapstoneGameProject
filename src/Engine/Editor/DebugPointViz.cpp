#include "DebugPointViz.h"
#include <imgui.h>
#include <functional>
#include <string_view>
#include "../Core/GameObjectBase.h"
#include "../Managers/SpriteBatch.h"

namespace ETG
{
    // Green is deliberately missing: DrawOriginPoint already owns it, and a point authored right on top of an
    // origin has to stay readable as a separate thing.
    static constexpr ETG::Color MarkerPalette[]{
        {255, 0, 255}, // magenta
        {0, 255, 255}, // cyan
        {255, 255, 0}, // yellow
        {255, 64, 64}, // red
        {255, 144, 0}, // orange
        {140, 140, 255}, // periwinkle
        {255, 128, 200}, // pink
        {180, 255, 255}, // pale cyan
    };

    ETG::Color DebugPointViz::ColorFor(const char* label)
    {
        // Hashed from the member name rather than handed out in insertion order: with insertion order, turning one
        // marker off would recolor every marker that came after it.
        const std::size_t hash = std::hash<std::string_view>{}(label);
        return MarkerPalette[hash % std::size(MarkerPalette)];
    }

    void DebugPointViz::PushOwner(GameObjectBase* owner) { OwnerStack.push_back(owner); }

    void DebugPointViz::PopOwner()
    {
        if (!OwnerStack.empty()) OwnerStack.pop_back();
    }

    GameObjectBase* DebugPointViz::CurrentOwner()
    {
        return OwnerStack.empty() ? nullptr : OwnerStack.back();
    }

    void DebugPointViz::Toggle(const char* label, ETG::Vector2f& value, GameObjectBase* owner)
    {
        // Without an owner there is nothing to resolve the point against, so there is nothing worth drawing
        if (!owner) return;

        const void* key = &value;
        bool enabled = Markers.contains(key);
        const ETG::Color color = ColorFor(label);

        ImGui::PushID(key);

        // The swatch comes first because with several markers up at once the color is the only thing tying a cross
        // on screen back to the row that spawned it
        ImGui::ColorButton("##swatch",
                           ImVec4(color.r / 255.f, color.g / 255.f, color.b / 255.f, 1.f),
                           ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop,
                           ImVec2(ImGui::GetFrameHeight() * 0.6f, ImGui::GetFrameHeight() * 0.6f));
        ImGui::SameLine(0.f, 3.f);

        if (ImGui::Checkbox("##viz", &enabled))
        {
            if (enabled) Markers[key] = Marker{owner, &value, color};
            else Markers.erase(key);
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Visualize this point in the world");

        ImGui::PopID();
    }

    void DebugPointViz::DrawAll()
    {
        for (auto it = Markers.begin(); it != Markers.end();)
        {
            const Marker& marker = it->second;

            // The value lives inside the owner, so an owner that has been freed took the value with it. This is
            // the only check standing between a stale checkbox and a read of destroyed memory.
            if (!GameClass::IsValid(marker.Owner))
            {
                it = Markers.erase(it);
                continue;
            }

            SpriteBatch::DrawDebugCross(marker.Owner->ResolveDebugPoint(*marker.Value), marker.Color);
            ++it;
        }
    }
}
