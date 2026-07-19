#pragma once
#include "../Platform/Platform.h"
#include "../Platform/Platform.h"
#include <memory>
#include <boost/describe.hpp>
#include "GameClass.h"
#include "TypeID.h"
#include "../../Utils/Interface/IAnimationComponent.h"

namespace ETG
{
    class GameObjectBase : public GameClass
    {
    public:
        struct DrawProperties
        {
            ETG::Vector2f Position{0, 0};
            ETG::Vector2f Scale{1, 1};
            ETG::Vector2f Origin{0, 0};
            float Rotation{};
            float Depth{};
            ETG::Color Color{ETG::Color::White};
            ETG::Texture* Texture = nullptr;
        };

    public:
        //Public so the central scene list (vector<unique_ptr<GameObjectBase>>) can update/draw/destroy polymorphically
        virtual ~GameObjectBase();
        virtual void Initialize();
        virtual void Draw();
        virtual void Update();

    protected:
        //Push back every GameObject to the SceneObj during initialization.
        GameObjectBase();
        // TypeID::IDType SetTypeID();

        //Base position of GameObjects
        //Inherited Objects such as Gun's position will be attached to hand pos in tick. After the object manipulations are completed, the relative offsets needs given in UI needs to be applied
        //and result will be stored in FinalPos, FinalRot etc. Final properties will be drawn.    
        ETG::Vector2f Position{0, 0};
        ETG::Vector2f Scale = {1, 1};
        float Rotation{};
        ETG::Vector2f Origin{0.f, 0.f};
        ETG::Color Color{ETG::Color::White};
        float Depth{};

        //Destroy
        bool PendingDestroy = false;

        //NOTE: Pointer to animation component interface. This is my first time using interface logic
        IAnimationComponent* AnimInterface = nullptr;

    private:
        //Relative Offsets for GameObjects.
        ETG::Vector2f RelativePos{0.f, 0.f};
        ETG::Vector2f RelativeScale = {1, 1};
        float RelativeRotation{};
        ETG::Vector2f RelativeOrigin{0.f, 0.f};

        //Previous Relative Offsets
        ETG::Vector2f PrevRelativePos{0.f, 0.f};
        ETG::Vector2f PrevRelativeScale{0.f, 0.f};
        ETG::Vector2f PrevRelativeRot{0.f, 0.f};

        //Contains the final drawing properties. 
        DrawProperties DrawProps;

        //The typename without any increment
        std::string TypeName{};

        //Each object's type ID
        TypeID::IDType TypeID{};

    public:
        void ComputeDrawProperties();
        void VisualizeOrigin() const;
        void IncrementName();

        GameObjectBase* Owner = nullptr;
        bool DrawBound = false;
        bool DrawOriginPoint = false;
        bool IsGameObjectUISpecified = false;
        std::string ObjectName{"Default"};
        std::shared_ptr<ETG::Texture> Texture;
        bool IsVisible{true}; //For now I will only use this for Passive and Active item pick up.

        // Only the drawing code (or renderer) is expected to use these values.
        [[nodiscard]] const DrawProperties& GetDrawProperties() const { return DrawProps; }
        virtual std::string& GetObjectName() { return ObjectName; }
        [[nodiscard]] const std::string& GetTypeName() const { return TypeName; }
        [[nodiscard]] const GameObjectBase* GetOwner() const { return Owner; }

        [[nodiscard]] const ETG::Vector2f& GetPosition() const { return Position; }
        [[nodiscard]] float GetRotation() const { return Rotation; }
        [[nodiscard]] const ETG::Vector2f& GetScale() const { return Scale; }
        [[nodiscard]] const ETG::Vector2f& GetOrigin() const { return Origin; }
        [[nodiscard]] const ETG::Color& GetColor() const { return Color; }

        [[nodiscard]] const ETG::Vector2f& GetRelativePosition() const { return RelativePos; }
        [[nodiscard]] const ETG::Vector2f& GetRelativeScale() const { return RelativeScale; }
        [[nodiscard]] const ETG::Vector2f& GetRelativeOrigin() const { return RelativeOrigin; }

        [[nodiscard]] TypeID::IDType GetType() const { return TypeID; }


        virtual void SetPosition(const ETG::Vector2f& Position) { this->Position = Position; }
        void SetRotation(const float& rotation) { this->Rotation = rotation; }
        void SetScale(const ETG::Vector2f& Scale) { this->Scale = Scale; }
        void SetOrigin(const ETG::Vector2f& Origin) { this->Origin = Origin; }
        void SetColor(const ETG::Color& color) { this->Color = color; }

        //Mark this object to be destroyed
        virtual void MarkForDestroy() { PendingDestroy = true; }
        [[nodiscard]] bool IsPendingDestroy() const { return PendingDestroy; }
        bool IsValid() const { return GameClass::IsValid(this); }

        // Animation component management
        void SetAnimationInterface(IAnimationComponent* animComp) { AnimInterface = animComp; }
        [[nodiscard]] IAnimationComponent* GetAnimationInterface() const { return AnimInterface; } //Never used yet

        // Bounds methods
        [[nodiscard]] ETG::FloatRect GetBounds() const;
        void DrawBounds(ETG::Color color = ETG::Color::Red) const;

        //If same named object constructed before, differentiate it with appending a number end of the name
        //ex: BaseProjectile BaseProjectile2 BaseProjectile3 
        std::string SetObjectNameToSelfClassName();

        virtual void PopulateSpecificWidgets();

        //Friend classes for Engine UI
        friend void ImGuiSetRelativeOrientation(GameObjectBase* obj);
        friend void ImGuiSetAbsoluteOrientation(GameObjectBase* obj);

        //NOTE:----------------------Type ID----------------------------
        // Setter for the type ID (to be called from factory)
        template <typename T>
        void SetTypeInfo()
        {
            TypeID = TypeID::GetID<T>();
        }

        //Type checking without knowing derived types
        template <typename T>
        [[nodiscard]] bool IsA() const
        {
            return TypeID::IsBaseOf(GetType(), TypeID::GetID<T>());
        }

        //Safe casting
        template <typename T>
        T* As()
        {
            return IsA<T>() ? static_cast<T*>(this) : nullptr;
        };

        template <typename T>
        const T* As() const
        {
            return IsA<T>() ? static_cast<const T*>(this) : nullptr;
        }

        //Check owner hierarchy
        template <typename T>
        bool HasOwnerOfType(int levels = 1) const
        {
            if (levels <= 0 || !Owner) return false;
            if (Owner->IsA<T>()) return true;
            return levels > 1 && Owner->HasOwnerOfType<T>(levels - 1);
        }

        BOOST_DESCRIBE_CLASS(GameObjectBase, (GameClass),
                             (Owner, ObjectName,Texture, DrawOriginPoint, DrawBound, IsVisible),
                             (Origin, Depth),
                             ())
    };
}
