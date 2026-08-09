#pragma once
#include <functional>
#include <memory>
#include <imgui.h>
#include "../ComponentBase.h"
#include "../Direction.h"
#include "../../Animation/AnimationManager.h"
#include "../../Animation/IAnimationComponent.h"
#include "../../Editor/UI/UIUtils.h"
#include "../../../Utils/StrManipulateUtil.h"

//This class looks ugly however because it's heavy templated class, I cannot use one by one write all possible template specializations it will be over 100+, so there's nothing I can do
//NOTE: An animation meant to manage animations, NOT TO MODIFY OBJECT ORIENTATION PROPERTIES 
namespace ETG
{

    template <typename StateEnum>
    class BaseAnimComp : public ComponentBase, public IAnimationComponent
    {
    public:
        enum class FlipAxis
        {
            X,
            Y,
            Both
        };

        // Override the base class Initialize method to register with owner
        void Initialize() override;

        virtual void SetAnimations();
        void Update(const StateEnum& stateEnum, const AnimationKey& animKey);
        void PopulateSpecificWidgets() override;


    public:
        //Given the owner's current state, which sub-animation (usually a direction) should play.
        //NOTE: This replaces the switch each derived anim component used to carry. The mapping is data now, so a new
        //state is one registration next to its animations instead of another case label in another switch
        using KeyResolver = std::function<AnimationKey()>;

        void SetKeyResolver(StateEnum state, KeyResolver resolver) { KeyResolvers[state] = std::move(resolver); }

        //Falls back to the key currently playing when a state has no resolver registered
        [[nodiscard]] AnimationKey ResolveKey(const StateEnum& state) const
        {
            const auto it = KeyResolvers.find(state);
            return it != KeyResolvers.end() ? it->second() : CurrentAnimStateKey;
        }

        template <typename DirectionEnum>
        void AddAnimationsForState(StateEnum state, Playback playback, const std::vector<Animation>& animations);

        void AddGunAnimationForState(StateEnum state, Playback playback, const Animation& animation, bool IsManualOrigin = false, ETG::Vector2f origin = {});

        // Implement IAnimationComponent interface
        [[nodiscard]] ETG::IntRect GetCurrentTextureRect() const override { return CurrTexRect; }
        [[nodiscard]] const Animation* GetAnimation() const override {return GetCurrentAnimation();};
        [[nodiscard]] const Animation* GetCurrentAnimation() const;

        template <typename... TObjects>
        void FlipSprites(const Direction& currentDirection, FlipAxis axis, TObjects&... objects);

        template <typename... TObjects>
        void FlipSpritesX(const Direction& currentDirection, TObjects&... objects);

        template <typename... TObjects>
        void FlipSpritesY(const Direction& currentDirection, TObjects&... objects);

        //If the key has changed, change the animation state and restart
        void ChangeAnimStateIfRequired(const AnimationKey& newKey);
        
        //Animation properties
        std::unordered_map<StateEnum, AnimationManager> AnimManagerDict{};
        std::unordered_map<StateEnum, KeyResolver> KeyResolvers{};

        //Value-initialized on purpose. Owners read this before the first Update writes it - GunBase::Update asks
        //"is the current state's animation finished?" at the top of the frame - and an uninitialized enum answered
        //with whatever the heap happened to hold. Zero is the first enumerator, which is the idle state in every
        //enum this template is used with.
        StateEnum CurrentState{};
        AnimationKey CurrentAnimStateKey;

    private:
        //These private fields just to be displayed in UI. Do not consider to make them public to access something. If there's a variable at here that you want to access, there'll always be a way other than making this public
        std::shared_ptr<ETG::Texture> CurrentTex;
        ETG::IntRect CurrTexRect;

        BOOST_DESCRIBE_CLASS(BaseAnimComp, (ComponentBase), (CurrentState), (), (CurrentTex, CurrTexRect))
    };

    //-------------------------------------------------------------Definition-------------------------------------------------------------
    template <typename StateEnum>
    void BaseAnimComp<StateEnum>::Update(const StateEnum& stateEnum, const AnimationKey& animKey)
    {
        //The state has to be current before the restart below, so the restart hits the new state's manager
        CurrentState = stateEnum;

        //NOTE: This has to run BEFORE the lookups below, and CurrentAnimStateKey must not be assigned beforehand.
        //It used to be the other way around: the key was assigned first and this was called last, which made its
        //`newKey != CurrentAnimStateKey` check permanently false. The restart never fired, which is why callers had
        //to restart animations by hand (HeroAnimComp::StartDash and its hit handler both did)
        ChangeAnimStateIfRequired(animKey);

        //A state nobody registered animations for is not played, and above all does not get CREATED here. These two
        //lookups used to be operator[], which inserted an empty manager holding a default-constructed Animation - an
        //Animation with no texture. That nullptr went straight into Owner->Texture below and killed the first thing
        //that read it a frame later, as far from the cause as it gets: the crash landed in the UI's gun frame while
        //the real mistake was a gun asking for a state it never authored.
        const auto managerIt = AnimManagerDict.find(CurrentState);
        if (managerIt == AnimManagerDict.end()) return;

        AnimationManager& animManager = managerIt->second;
        animManager.Update(CurrentAnimStateKey);

        const auto animIt = animManager.AnimationDict.find(CurrentAnimStateKey);
        if (animIt == animManager.AnimationDict.end()) return;

        const Animation& animState = animIt->second;
        CurrTexRect = animState.CurrRect;
        CurrentTex = animState.GetCurrentFrameAsTexture();
        Owner->Texture = CurrentTex;
        Owner->SetOrigin(animState.Origin);
    }

    template <typename StateEnum>
    void BaseAnimComp<StateEnum>::Initialize()
    {
        ComponentBase::Initialize();

        if (!Owner) throw std::runtime_error("Owner cannot be empty. Every animation should be an owner game object.");

        // Register with owner
        Owner->SetAnimationInterface(this);
    }

    template <typename StateEnum>
    void BaseAnimComp<StateEnum>::SetAnimations()
    {
    }

    template <typename StateEnum>
    template <typename DirectionEnum>
    void BaseAnimComp<StateEnum>::AddAnimationsForState(StateEnum state, const Playback playback, const std::vector<Animation>& animations)
    {
        auto animManager = AnimationManager{};
        std::vector<DirectionEnum> enumKeys = ConstructEnumVector<DirectionEnum>();

        // Make sure we don't exceed the bounds of either array
        const size_t count = std::min(enumKeys.size(), animations.size());

        for (size_t i = 0; i < count; ++i)
        {
            Animation anim = animations[i];
            anim.Loops = playback == Playback::Loop;
            animManager.AddAnimation(enumKeys[i], anim);

            // Only set the origin if the animation has valid frames
            if (!animations[i].FrameRects.empty())
            {
                animManager.SetOrigin(enumKeys[i], ETG::Vector2f{
                                          (float)animations[i].FrameRects[0].width / 2, //x
                                          (float)animations[i].FrameRects[0].height / 2 //y
                                      });
            }
        }

        AnimManagerDict[state] = animManager;
    }

    template <typename StateEnum>
    void BaseAnimComp<StateEnum>::AddGunAnimationForState(StateEnum state, const Playback playback, const Animation& animation, const bool IsManualOrigin, ETG::Vector2f origin)
    {
        auto animManager = AnimationManager{};
        Animation anim = animation;
        anim.Loops = playback == Playback::Loop;
        animManager.AddAnimation(state, anim); // Using the state enum itself as the key

        if (!animation.FrameRects.empty())
        {
            //If manually origin is given, set the given origin for the current animation
            if (IsManualOrigin)
            {
                animManager.SetOrigin(state, origin);
            }
            //IF manually origin is not given, by default set the origin to be the center of the first frame
            else
            {
                animManager.SetOrigin(state, ETG::Vector2f{
                                          (float)animation.FrameRects[0].width / 2,
                                          (float)animation.FrameRects[0].height / 2
                                      });
            }
        }

        AnimManagerDict[state] = animManager;
    }

    template <typename StateEnum>
    template <typename... TObjects>
    void BaseAnimComp<StateEnum>::FlipSprites(const Direction& currentDirection, FlipAxis axis, TObjects&... objects)
    {
        if (!AnimManagerDict.contains(CurrentState))
            throw std::runtime_error("CurrentState not found in the AnimManagerDict");

        //NOTE: ETG::IsFacingRight, next to the Direction enum itself. It used to be a static on this class that
        //substring-matched the enum's *name* for "Right", which is what forced Direction to spell out a side
        bool facingRight = ETG::IsFacingRight(currentDirection);

        bool flipX = (axis == FlipAxis::X) || axis == FlipAxis::Both;
        bool flipY = (axis == FlipAxis::Y) || axis == FlipAxis::Both;

        //Logic is simple. If facing right, let the scale be 1.0, if it is not facing right, make -1.0
        auto flipObjectScale = [facingRight, flipX, flipY](auto& obj)
        {
            ETG::Vector2f scale = obj.GetScale();
            if (flipX) scale.x = facingRight ? std::abs(scale.x) : -std::abs(scale.x);
            if (flipY) scale.y = facingRight ? std::abs(scale.y) : -std::abs(scale.y);

            obj.SetScale(scale);
        };

        (flipObjectScale(objects), ...);
    }

    template <typename StateEnum>
    template <typename... TObjects>
    void BaseAnimComp<StateEnum>::FlipSpritesX(const Direction& currentDirection, TObjects&... objects)
    {
        return FlipSprites(currentDirection, FlipAxis::X, objects...);
    }

    template <typename StateEnum>
    template <typename... TObjects>
    void BaseAnimComp<StateEnum>::FlipSpritesY(const Direction& currentDirection, TObjects&... objects)
    {
        return FlipSprites(currentDirection, FlipAxis::Y, objects...);
    }

    template <typename StateEnum>
    void BaseAnimComp<StateEnum>::ChangeAnimStateIfRequired(const AnimationKey& newKey)
    {
        //If previous animation state (ex: idle) and current (ex: run) is different, update key and restart the animation
        if (newKey != CurrentAnimStateKey)
        {
            CurrentAnimStateKey = newKey;

            //Restart the key
            auto& animManager = AnimManagerDict[CurrentState];
            if (animManager.AnimationDict.contains(CurrentAnimStateKey))
            {
                animManager.AnimationDict[CurrentAnimStateKey].Restart();
            }
        }
    }

    template <typename StateEnum>
    const Animation* BaseAnimComp<StateEnum>::GetCurrentAnimation() const
    {
        return AnimManagerDict.at(CurrentState).GetCurrentAnimation();
    }


    //-----------------------------------------UI----------------------------------------
    template <typename StateEnum>
    void BaseAnimComp<StateEnum>::PopulateSpecificWidgets()
    {
        ComponentBase::PopulateSpecificWidgets();

        // Display CurrentState
        UIUtils::BeginProperty("Current State");
        ImGui::Text("%s", EnumToString<StateEnum>(CurrentState));
        UIUtils::EndProperty();

        // Display CurrentAnimStateKey
        UIUtils::BeginProperty("Current Animation Key");
        UIUtils::DisplayAnimationKey(CurrentAnimStateKey);
        UIUtils::EndProperty();

        // Display CurrentTex (texture preview)
        UIUtils::BeginProperty("Current Texture");
        UIUtils::DisplayTexture(CurrentTex);
        UIUtils::EndProperty();

        // Display CurrTexRect
        if (ImGui::TreeNode("Texture Rectangle##CurrTexRect"))
        {
            UIUtils::DisplayIntRectangle(CurrTexRect);
            ImGui::TreePop();
        }

        // Display AnimManagerDict
        if (ImGui::TreeNode("Animation Managers Dictionary"))
        {
            ImGui::Text("Size: %zu animation managers", AnimManagerDict.size());
            ImGui::Separator();

            // Iterate through AnimManagerDict and display each entry
            int index = 0;
            for (auto& [stateEnum, animManager] : AnimManagerDict)
            {
                ImGui::PushID(index++);

                std::string FirstStr = EnumToString(stateEnum);
                std::string stateLabel = "State: " + FirstStr;
                bool isCurrentState = (stateEnum == CurrentState);

                // Highlight current state
                if (isCurrentState) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow for current state

                if (ImGui::TreeNode(stateLabel.c_str()))
                {
                    // Current state indicator
                    UIUtils::BeginProperty("Is Current State");
                    ImGui::BeginDisabled(true);
                    ImGui::Checkbox("##IsCurrent", &isCurrentState);
                    ImGui::EndDisabled();
                    UIUtils::EndProperty();

                    // Display AnimationManager for this state
                    UIUtils::DisplayAnimationManager("Animation Manager", animManager);

                    ImGui::TreePop();
                }

                if (isCurrentState)
                {
                    ImGui::PopStyleColor();
                }

                ImGui::PopID();
            }

            ImGui::TreePop();
        }
    }
}
