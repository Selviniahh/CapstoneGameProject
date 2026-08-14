#pragma once
#include "../Platform/Platform.h"
#include <memory>
#include <boost/describe.hpp>
#include "GameClass.h"
#include "TypeID.h"
#include "../Animation/IAnimationComponent.h"

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
            //Which fragment program the renderer submits this object with. Characters override it
            //to ShaderEffect::Grayscale; everything else stays on the plain sprite shader.
            ETG::ShaderEffect Effect{ETG::ShaderEffect::None};
            //The vec4 handed to that program, per object (see ETG::ShaderEffectParams).
            ETG::ShaderEffectParams EffectParams{};
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

        //Where every AddListener call in the game belongs. The rule, in full, is in docs/InitializationRules.tr.md;
        //the short version is three lines:
        //
        //  1. A class binds its OWN listeners here and nothing else's.
        //  2. Its OWN constructor calls its OWN qualified version - Hero::BindEvents() from Hero's constructor -
        //     as the last statement. A constructor runs exactly once per object, which is the entire point.
        //  3. An override NEVER calls Base::BindEvents(). The base constructor already ran its own, so chaining
        //     would bind it a second time.
        //
        //Initialize() is the wrong place and used to be the place: it is called by the class' own constructor AND
        //again by every derived constructor, and EventDelegate::AddListener happily takes the same lambda twice.
        //EnemyBase paid for that with two ApplyDamage calls per bullet and two OnDeath broadcasts per death.
        //
        //Deliberately never called through a base pointer either: dispatching would reach only the most derived
        //override and silently skip every base class' bindings
        virtual void BindEvents()
        {
        }

        //Base position of GameObjects
        //Inherited Objects such as Gun's position will be attached to hand pos in tick. After the object manipulations are completed, the relative offsets needs given in UI needs to be applied
        //and result will be stored in FinalPos, FinalRot etc. Final properties will be drawn.    
        ETG::Vector2f Position{0, 0};
        ETG::Vector2f Scale = {1, 1};
        float Rotation{};
        ETG::Vector2f Origin{0.f, 0.f};
        ETG::Color Color{ETG::Color::White};
        float Depth{};

        //The shader this object's sprite goes through, and the parameters it is given. Set by whoever
        //wants it - Character asks for grayscale in its constructor, ShaderEffectComponent swaps it
        //for the length of a hit flash - and copied into DrawProps every frame.
        ETG::ShaderEffect Effect{ETG::ShaderEffect::None};
        ETG::ShaderEffectParams EffectParams{};

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

        // Where a described ETG::Vector2f member actually sits when the editor's Visualize toggle draws it.
        // `label` is the member's own name, so a class can answer differently for each of its members: an
        // authored grip point, an origin nudge and a plain world position all live in the same object.
        //
        // The default hands the point to the Owner. Nearly every authored offset in the game is written in the
        // frame of the thing it is attached to - ShellEjector's ejection port, HandRig's anchors, AK47's magazine
        // point are all gun-local - and each of those is its own scene object, so without the delegation their
        // markers would be resolved against nothing and land in the map's corner. The chain ends at whoever owns
        // the frame (GunBase below), or at an unowned object, whose points are world positions already.
        [[nodiscard]] virtual ETG::Vector2f ResolveDebugPoint(const char* label, const ETG::Vector2f& point) const;

        // The frames an authored ETG::Vector2f can be written in. Overrides pick one per member instead of
        // repeating the rotate-and-mirror arithmetic:
        //   Local        - an offset from this object's Position, turning and mirroring with it.
        //   Texture      - a pixel read off this object's sprite sheet, in that frame's own coordinates.
        //   OriginShift  - a nudge added to Origin. The pivot keeps landing on Position and the artwork slides
        //                  the other way, so the marker follows the artwork: that is what the value moves.
        [[nodiscard]] ETG::Vector2f LocalDebugPoint(const ETG::Vector2f& offset) const;
        [[nodiscard]] ETG::Vector2f TextureDebugPoint(const ETG::Vector2f& texel) const;
        [[nodiscard]] ETG::Vector2f OriginShiftDebugPoint(const ETG::Vector2f& shift) const;

        // Labels arrive as string literals from the reflection walk, so matching one is a strcmp. Spelled out
        // once here rather than at every branch of every override.
        [[nodiscard]] static bool DebugLabelIs(const char* label, const char* name);

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

        //The shader this object draws with. Public because it is not only the object itself that decides:
        //ShaderEffectComponent takes it over for the length of a hit flash and puts the old one back after
        [[nodiscard]] ETG::ShaderEffect GetEffect() const { return Effect; }
        [[nodiscard]] const ETG::ShaderEffectParams& GetEffectParams() const { return EffectParams; }
        void SetEffect(const ETG::ShaderEffect effect, const ETG::ShaderEffectParams& params = {})
        {
            Effect = effect;
            EffectParams = params;
        }

        //Pushes the current effect into draw properties that were already published this frame.
        //
        //Every other field is copied across once per frame, in ComputeDrawProperties, from inside the owner's own
        //Update - which is fine for anything the owner decides for itself. The effect is not always one of those:
        //a hit flash is started by the collision pass, and collision runs after every object's Update has finished
        //(see CollisionSystem). Without this, an effect switched on by a collision would miss the draw it belongs
        //to and only appear on the following frame - and a flash shorter than a frame would be aged out before it
        //was ever drawn, which is the exact case ShaderEffectComponent::MinFramesShown exists to rule out.
        //
        //Deliberately only the two effect fields, not a full ComputeDrawProperties: republishing everything would
        //also pull in a position the object has moved to since it published, and objects do not agree on where in
        //their Update they publish
        void RepublishEffect()
        {
            DrawProps.Effect = Effect;
            DrawProps.EffectParams = EffectParams;
        }

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
