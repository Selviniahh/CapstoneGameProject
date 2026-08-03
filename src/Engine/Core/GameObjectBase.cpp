#include "GameObjectBase.h"
#include "Scene/Scene.h"
#include <boost/type_index.hpp>
#include <imgui.h>
#include <iostream>

#include "Factory.h"
#include "../Managers/SpriteBatch.h"
#include "../../Utils/StrManipulateUtil.h"


ETG::GameObjectBase::GameObjectBase()
{
    GameObjectBase::Initialize();
}

ETG::GameObjectBase::~GameObjectBase()
{
    //Since the pointer doesn't own the memory, I don't need to bother with memory leak for the pointer. However just in case I set it to nullptr in destructor
    AnimInterface = nullptr;
    UnregisterGameObject(this);
}

void ETG::GameObjectBase::Initialize()
{
    if (Texture.get())
    {
        Origin.x = Texture->getSize().x / 2;
        Origin.y = Texture->getSize().y / 2;
    }
    ComputeDrawProperties();
}

void ETG::GameObjectBase::VisualizeOrigin() const
{
    if (DrawOriginPoint)
    {
        SpriteBatch::DrawSinglePixelAtLoc(DrawProps.Position, ETG::Vector2i(1, 1), Rotation);
    }
}

void ETG::GameObjectBase::Draw()
{
    if (!IsVisible) return;
    VisualizeOrigin();
    DrawBounds();
    SpriteBatch::Draw(GetDrawProperties()); //Let's see
}

void ETG::GameObjectBase::Update()
{
    //Calculate the final drawing properties. The Base position modified from the source and relative pos given in the UI will be summed to form the final value before being drawn
    ComputeDrawProperties();
}

void ETG::GameObjectBase::ComputeDrawProperties()
{
    DrawProps.Position = Position + RelativePos;
    DrawProps.Scale = {Scale.x * RelativeScale.x, Scale.y * RelativeScale.y};
    DrawProps.Rotation = Rotation + RelativeRotation;
    DrawProps.Origin = Origin + RelativeOrigin;
    DrawProps.Depth = Depth;
    DrawProps.Texture = Texture.get();
    DrawProps.Color = Color;
    DrawProps.Effect = Effect;
}

ETG::FloatRect ETG::GameObjectBase::GetBounds() const
{
    if (AnimInterface)
    {
        // Use animation component's texture rect for bounds
        const auto& texRect = AnimInterface->GetCurrentTextureRect();
        return {
            Position.x - texRect.width / 2.f,
            Position.y - texRect.height / 2.f,
            static_cast<float>(texRect.width),
            static_cast<float>(texRect.height)
        };
    }
    else if (Texture)
    {
        // Fallback to using the texture size if available
        return {
            Position.x - Texture->getSize().x / 2.f, //X coordinate of the top-left corner
            Position.y - Texture->getSize().y / 2.f, //Y coordinate of the top-left corner
            static_cast<float>(Texture->getSize().x), //usual width size
            static_cast<float>(Texture->getSize().y) //usual height size
        };
    }

    // Default to a small square if no texture information is available
    return {
        Position.x - 5.f,
        Position.y - 5.f,
        10.f,
        10.f
    };
}

void ETG::GameObjectBase::DrawBounds(ETG::Color color) const
{
    if (!DrawBound) return;
    GlobSpriteBatch.drawRectOutline(GetBounds(), color, 1.0f, 0);
}

std::string ETG::GameObjectBase::SetObjectNameToSelfClassName()
{
    ObjectName = boost::typeindex::type_id_runtime(*this).pretty_name();
    RemoveNamespaceFromName(ObjectName);

    TypeName = boost::typeindex::type_id<decltype(*this)>().pretty_name();
    RemoveNamespaceFromName(TypeName);

    IncrementName();

    return ObjectName;
}

void ETG::GameObjectBase::PopulateSpecificWidgets()
{
}

void ETG::GameObjectBase::IncrementName()
{
    //The scene itself is named before any registry exists; nothing to collide with yet
    if (!Scene::Get()) return;
    const auto& SceneObjs = Scene::Get()->SceneObjs;
    const std::string BaseName = ObjectName;

    const bool nameTaken = std::ranges::any_of(SceneObjs, [this, &BaseName](GameObjectBase* obj) { return obj != this && obj->GetObjectName() == BaseName; });
    if (nameTaken)
    {
        std::vector<signed int> Suffixes;
        Suffixes.push_back(1);

        for (auto* gameObject : SceneObjs)
        {
            const std::string& objectName = gameObject->GetObjectName();
            if (gameObject == this || objectName == ObjectName) continue;

            // Only process if it contains the base name and has potential numeric suffix
            if (objectName.starts_with(BaseName))
            {
                std::string Suffix = objectName.substr(BaseName.size());

                // Check if Suffix is purely numeric
                const bool isNumeric = !Suffix.empty() && std::ranges::all_of(Suffix,
                                                                              [](const char c) { return std::isdigit(c); });

                if (isNumeric)
                {
                    const signed int LastDigit = std::stoi(Suffix);
                    Suffixes.push_back(LastDigit);
                }
            }
        }

        const int max = std::ranges::max(Suffixes);
        ObjectName = BaseName + std::to_string(max + 1);
    }
}
