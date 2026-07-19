#pragma once

#include <string>
#include <typeindex>
#include <unordered_map>
#include <memory>
#include "Animation.h"
#include "../../Utils/StrManipulateUtil.h"
#include "boost/describe.hpp"

//Type-erased animation key: any described enum, int or string works as a key without the
//engine having to know the game's enum types (they used to be hardcoded in a std::variant here).
struct AnimationKey
{
    std::type_index Type{typeid(void)};
    long long Value{};
    std::string Name{}; //Human-readable form; also what the editor displays

    AnimationKey() = default;
    AnimationKey(const char* s) : Type(typeid(std::string)), Name(s) {}
    AnimationKey(std::string s) : Type(typeid(std::string)), Name(std::move(s)) {}
    AnimationKey(const int v) : Type(typeid(int)), Value(v), Name(std::to_string(v)) {}

    template <typename E, std::enable_if_t<std::is_enum_v<E>, int>  = 0>
    AnimationKey(E e) : Type(typeid(E)), Value(static_cast<long long>(e)), Name(ETG::EnumToString(e)) {}

    bool operator==(const AnimationKey& other) const
    {
        return Type == other.Type && Value == other.Value && Name == other.Name;
    }
};

struct AnimationKeyHash
{
    std::size_t operator()(const AnimationKey& key) const
    {
        return key.Type.hash_code() ^ (std::hash<long long>{}(key.Value) << 1) ^ (std::hash<std::string>{}(key.Name) << 2);
    }
};

struct AnimationKeyEqual
{
    bool operator()(const AnimationKey& lhs, const AnimationKey& rhs) const
    {
        return lhs == rhs;
    }
};

class AnimationManager : GameClass
{
public:
    using AnimationMap = std::unordered_map<AnimationKey, Animation, AnimationKeyHash, AnimationKeyEqual>;
    AnimationMap AnimationDict;

    // For storing whichever key was last used
    AnimationKey LastKey;
    Animation* CurrentAnim = nullptr;

    std::shared_ptr<ETG::Texture> LastTexture;

    // Add an animation to the dictionary
    template <typename T>
    void AddAnimation(T key, const Animation& animation);

    // Update a specific animation
    template <typename T>
    void Update(T key);

    // Draw the last key's animation
    // void Draw(ETG::Vector2f position, float layerDepth);

    // Overloaded draw for more complex parameters
    void Draw(const std::shared_ptr<ETG::Texture>& texture, ETG::Vector2f position, ETG::Color color, float rotation, ETG::Vector2f origin, ETG::Vector2f scale, float depth);

    // Optionally set origin
    template <typename T>
    void SetOrigin(T key, ETG::Vector2f origin);

    // Return the current frame as a texture for the last key
    std::shared_ptr<ETG::Texture> GetCurrentFrameAsTexture();
    const Animation* GetCurrentAnimation() const;

    // Check if the current animation has finished
    bool IsAnimationFinished();

    BOOST_DESCRIBE_CLASS(AnimationManager, (GameClass), (CurrentAnim, AnimationDict, LastKey), (), ())
};

// <---------- Implementation ---------->

template <typename T>
void AnimationManager::AddAnimation(T key, const Animation& animation)
{
    // Convert user-supplied key to AnimationKey
    const AnimationKey variantKey = key; // Will compile if T is int, std::string, or EnemyIdle
    AnimationDict[variantKey] = animation;
    LastKey = variantKey;
}

template <typename T>
void AnimationManager::Update(T key)
{
    const auto variantKey = AnimationKey(key);

    // If the animation key exists
    if (AnimationDict.contains(variantKey))
    {
        CurrentAnim = &AnimationDict[variantKey];
        LastTexture = CurrentAnim->Texture;
        CurrentAnim->Update();
        LastKey = variantKey;
    }
    else
    {
        // If the key doesn't exist, restart the last animation
        AnimationDict[LastKey].Restart();
        CurrentAnim = &AnimationDict[LastKey];
    }
}

inline void AnimationManager::Draw(const std::shared_ptr<ETG::Texture>& texture, const ETG::Vector2f position, const ETG::Color color, const float rotation, const ETG::Vector2f origin, const ETG::Vector2f scale, const float depth)
{
    auto it = AnimationDict.find(LastKey);
    if (it != AnimationDict.end())
    {
        it->second.Draw(texture, position, color, rotation, origin, scale, depth);
    }
}

template <typename T>
void AnimationManager::SetOrigin(T key, const ETG::Vector2f origin)
{
    const AnimationKey variantKey = key;
    const auto it = AnimationDict.find(variantKey);
    if (it != AnimationDict.end())
    {
        it->second.Origin = origin;
    }
}

inline std::shared_ptr<ETG::Texture> AnimationManager::GetCurrentFrameAsTexture()
{
    const auto it = AnimationDict.find(LastKey);
    if (it != AnimationDict.end())
    {
        return it->second.GetCurrentFrameAsTexture();
    }

    throw std::runtime_error("Failed to find current frame's texture. Last Key: ");
}

const inline Animation* AnimationManager::GetCurrentAnimation() const
{
    const auto it = AnimationDict.find(LastKey);
    if (it != AnimationDict.end())
    {
        return &it->second;
    }
    return nullptr;
}

inline bool AnimationManager::IsAnimationFinished()
{
    const auto it = AnimationDict.find(LastKey);
    if (it != AnimationDict.end())
    {
        return it->second.IsAnimationFinished();
    }
    return true; // Or false, depending on your preference
}
