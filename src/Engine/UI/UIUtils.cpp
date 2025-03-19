#include "UIUtils.h"
#include <imgui.h>
#include "../../Animation/Animation.h"
#include "../../Utils/StrManipulateUtil.h"

void UIUtils::DisplayIntRectangle(sf::IntRect& rect)
{
    BeginProperty("Size");
    int size[2] = {rect.width, rect.height};
    if (ImGui::InputInt2("##Size", size))
    {
        rect.width = size[0];
        rect.height = size[1];
    }
    EndProperty();

    BeginProperty("Position");
    int pos[2] = {rect.top, rect.height};
    if (ImGui::InputInt2("##Position", pos))
    {
        // Prevent negative dimensions
        rect.left = pos[0];
        rect.top = pos[1];
    }
    EndProperty();
}

//    BOOST_DESCRIBE_CLASS(Animation,(GameClass),(IsValid, flipX),(), ())
void UIUtils::DisplayAnimation(const char* label, Animation& value)
{
    if (ImGui::TreeNode(label)) // Use the provided label as the header
    {
        // Display Origin as its own property row.
        BeginProperty("Origin");
        ImGui::InputFloat2("##Origin", &value.Origin.x);
        EndProperty();

        // Display rectangle info as a collapsible section.
        if (ImGui::TreeNode("Current Rectangle"))
        {
            DisplayIntRectangle(value.CurrRect);
            ImGui::TreePop();
        }

        // Display Texture:
        BeginProperty("Texture");
        DisplayTexture(value.Texture);
        EndProperty();

        // Display input int:
        BeginProperty("Animation Speed ");
        ImGui::InputFloat("Animation", &value.EachFrameSpeed);
        EndProperty();

        BeginProperty("Flip X");
        ImGui::InputFloat("Flip X", &value.flipX);
        EndProperty();

        // Display Active checkbox as its own property row.
        //TODO: Consider to make this readonly
        BeginProperty("Active");
        ImGui::Checkbox("##Active", &value.Active);
        EndProperty();

        //IsValid readonly
        BeginProperty("Is Valid");
        ImGui::BeginDisabled(true);
        ImGui::Checkbox("##Checkbox", &value.IsValid);
        ImGui::EndDisabled();
        EndProperty();

        // value.FrameRects
        BeginProperty("Frame Rectangles");
        bool framesOpen = ImGui::TreeNode("##FrameRects");
        EndProperty(); // End the property row before expanding content

        if (framesOpen)
        {
            for (int i = 0; i < value.FrameRects.size(); i++)
            {
                std::string arrLabel = "Frame " + std::to_string(i);

                // Create a tree node for each frame
                if (ImGui::TreeNode(arrLabel.c_str()))
                {
                    DisplayIntRectangle(value.FrameRects[i]);
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}


void UIUtils::DisplayTexture(const std::shared_ptr<sf::Texture>& value)
{
    if (value)
    {
        // Get the native OpenGL texture handle from SFML
        const ImTextureID textId = value->getNativeHandle();

        float texWidth = static_cast<float>(value->getSize().x);
        float texHeight = static_cast<float>(value->getSize().y);

        ImGui::Text("%.0fx%.0f", texWidth, texHeight);

        //Scale the texture
        constexpr float MIN_DIMENSION = 64.0f;
        float scale = 1.0f;
        const float smallestDimension = std::max(texWidth, texHeight);
        while (smallestDimension * (scale + 1) < MIN_DIMENSION)
        {
            scale += 1.0f;
        }
        texWidth *= scale;
        texHeight *= scale;

        //Uv min 0,0 UV mac: 1 1 which means display full image
        ImVec2 uvMin(0.0f, 0.0f);
        ImVec2 uv_max(1.0f, 1.0f);

        ImGui::Image(textId, ImVec2(texWidth, texHeight), uvMin, uv_max);
    }
    else
    {
        ImGui::Text("No texture loaded");
    }
}

// Function to display the variant AnimationKey
void UIUtils::DisplayAnimationKey(const AnimationKey& key)
{
    std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        
        if constexpr (std::is_same_v<T, std::string>)
        {
            ImGui::Text("String: %s", arg.c_str());
        }
        else if constexpr (std::is_enum_v<T>)
        {
            auto enumType = ETG::EnumToString<T>(arg);
            ImGui::Text("%s ", enumType);
        }
        else if constexpr (std::is_same_v<T, int>)
        {
            ImGui::Text("Int: %d", arg);
        }
    }, key);
}

void UIUtils::DisplayAnimationManager(const char* label, AnimationManager& manager)
{
    if (ImGui::TreeNode(label))
    {
        DisplayAnimation("Current Anim", *manager.CurrentAnim);
        
        // Display LastKey
        BeginProperty("Last Key");
        DisplayAnimationKey(manager.LastKey);
        EndProperty();
        
        // Display AnimationDict
        if (ImGui::TreeNode("Animation Dictionary"))
        {
            // Show the number of animations
            ImGui::Text("Size: %zu animations", manager.AnimationDict.size());
            ImGui::Separator();
            
            // Display each animation in the dictionary
            int index = 0;
            for (auto& [key, animation] : manager.AnimationDict)
            {
                ImGui::PushID(index++);
                
                // Display the key as a tree node header
                if (ImGui::TreeNode("Animation Entry"))
                {
                    //Display different color if the key is current
                    bool isCurrent = (manager.CurrentAnim == &animation);
                    if (isCurrent) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(9.0f, 0.0f, 1.0f, 1.0f)); // Yellow for current state
                    
                    // Display the key
                    BeginProperty("Key");
                    DisplayAnimationKey(key);
                    EndProperty();
                    
                    // Display whether this is the current animation
                    BeginProperty("Is Current");
                    

                    ImGui::BeginDisabled(true);
                    ImGui::Checkbox("##IsCurrent", &isCurrent);
                    ImGui::EndDisabled();
                    EndProperty();
                    
                    // Display the animation itself
                    std::string animLabel = "Animation Data";
                    DisplayAnimation(animLabel.c_str(), animation);

                    if (isCurrent) ImGui::PopStyleColor();
                    ImGui::TreePop();
                }
                
                ImGui::PopID();
            }
            
            ImGui::TreePop();
        }
        
        ImGui::TreePop();
    }
}

void UIUtils::DisplayColorPicker(const char* label, sf::Color& color)
{
    // Convert sf::Color (0-255) to ImGui color format (0.0f-1.0f)
    float colorArray[4] = {
        color.r / 255.0f,
        color.g / 255.0f,
        color.b / 255.0f,
        color.a / 255.0f
    };
    
    // Set up flags for the color picker
    ImGuiColorEditFlags flags = 
        ImGuiColorEditFlags_DisplayRGB |     // Display RGB values
        ImGuiColorEditFlags_DisplayHSV |     // Display HSV values
        ImGuiColorEditFlags_PickerHueWheel | // Show hue as a wheel
        ImGuiColorEditFlags_NoSidePreview;   // Hide side preview

    bool showAlpha = true;
    
    // Add alpha control if requested
    if (showAlpha) {
        flags |= ImGuiColorEditFlags_AlphaBar;
    } else {
        flags |= ImGuiColorEditFlags_NoAlpha;
    }
    
    // Begin a popup when clicked
    if (ImGui::ColorButton(label, ImVec4(colorArray[0], colorArray[1], colorArray[2], colorArray[3]), flags))
    {
        ImGui::OpenPopup(label);
    }
    
    // Show tooltip when hovered
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("RGB: (%d, %d, %d)", color.r, color.g, color.b);
        if (showAlpha) {
            ImGui::Text("Alpha: %d", color.a);
        }
        ImGui::EndTooltip();
    }
    
    ImGui::SameLine();
    ImGui::TextUnformatted(label);
    
    // Show the actual color picker in a popup
    if (ImGui::BeginPopup(label))
    {
        ImGui::Text("Color Picker");
        ImGui::Separator();
        
        bool changed = ImGui::ColorPicker4("##picker", colorArray, flags);
        
        if (changed)
        {
            color.r = static_cast<sf::Uint8>(colorArray[0] * 255.0f + 0.5f);
            color.g = static_cast<sf::Uint8>(colorArray[1] * 255.0f + 0.5f);
            color.b = static_cast<sf::Uint8>(colorArray[2] * 255.0f + 0.5f);
            color.a = static_cast<sf::Uint8>(colorArray[3] * 255.0f + 0.5f);
        }
        
        ImGui::EndPopup();
    }
}

void UIUtils::BeginProperty(const char* label)
{
    ImGui::PushID(label);
    ImGui::Columns(2);

    ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() * 0.40f);

    // Reduce vertical spacing
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, ImGui::GetStyle().ItemSpacing.y * 0.8f));

    ImGui::AlignTextToFramePadding();

    ImGui::Text("%s", label);

    ImGui::NextColumn();
    ImGui::SetNextItemWidth(-1);
}

void UIUtils::EndProperty()
{
    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();

    // Reduce spacing between properties
    ImGui::Spacing();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2); // Slightly reduce space between items
}
