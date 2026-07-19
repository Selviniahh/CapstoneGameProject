#pragma once
#include <memory>
#include <imgui.h>
#include "../ComponentBase.h"
#include "../Direction.h"
#include "../../Animation/AnimationManager.h"
#include "../../../Utils/Interface/IAnimationComponent.h"
#include "../../Editor/UI/UIUtils.h"
#include "../../../Utils/StrManipulateUtil.h"

//This class looks ugly however because it's heavy templated class, I cannot use one by one write all possible template specializations it will be over 100+, so there's nothing I can do
//NOTE: An animation meant to manage animations, NOT TO MODIFY OBJECT ORIENTATION PROPERTIES 
namespace ETG
{
    enum class HeroStateEnum;

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
        template <typename DirectionEnum>
        void AddAnimationsForState(StateEnum state, const std::vector<Animation>& animations);

        void AddGunAnimationForState(StateEnum state, const Animation& animation, bool IsManualOrigin = false, ETG::Vector2f origin = {});

        // Implement IAnimationComponent interface
        [[nodiscard]] ETG::IntRect GetCurrentTextureRect() const override { return CurrTexRect; }
        [[nodiscard]] const Animation* GetAnimation() const override {return GetCurrentAnimation();};
        [[nodiscard]] const Animation* GetCurrentAnimation() const;

        //I know this function is bad. I will think of something
        static bool IsFacingRight(const Direction& currentDirection);

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
        StateEnum CurrentState;
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
        CurrentState = stateEnum;
        CurrentAnimStateKey = animKey;

        auto& animManager = AnimManagerDict[CurrentState];
        animManager.Update(CurrentAnimStateKey);

        const auto& animState = animManager.AnimationDict[CurrentAnimStateKey];
        CurrTexRect = animState.CurrRect;
        CurrentTex = animState.GetCurrentFrameAsTexture();
        Owner->Texture = CurrentTex;
        Owner->SetOrigin(animManager.AnimationDict[CurrentAnimStateKey].Origin);

        ChangeAnimStateIfRequired(animKey);
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
    void BaseAnimComp<StateEnum>::AddAnimationsForState(StateEnum state, const std::vector<Animation>& animations)
    {
        auto animManager = AnimationManager{};
        std::vector<DirectionEnum> enumKeys = ConstructEnumVector<DirectionEnum>();

        // Make sure we don't exceed the bounds of either array
        const size_t count = std::min(enumKeys.size(), animations.size());

        for (size_t i = 0; i < count; ++i)
        {
            animManager.AddAnimation(enumKeys[i], animations[i]);

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
    void BaseAnimComp<StateEnum>::AddGunAnimationForState(StateEnum state, const Animation& animation, const bool IsManualOrigin, ETG::Vector2f origin)
    {
        auto animManager = AnimationManager{};
        animManager.AddAnimation(state, animation); // Using the state enum itself as the key

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
    bool BaseAnimComp<StateEnum>::IsFacingRight(const Direction& currentDirection)
    {
        return std::string{EnumToString(currentDirection)}.contains("Right") || std::string{EnumToString(currentDirection)}.contains("right");
    }

    template <typename StateEnum>
    template <typename... TObjects>
    void BaseAnimComp<StateEnum>::FlipSprites(const Direction& currentDirection, FlipAxis axis, TObjects&... objects)
    {
        if (!AnimManagerDict.contains(CurrentState))
            throw std::runtime_error("CurrentState not found in the AnimManagerDict");

        bool facingRight = IsFacingRight(currentDirection);

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
