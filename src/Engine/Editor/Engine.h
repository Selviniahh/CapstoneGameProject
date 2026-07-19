#pragma once
#include <unordered_set>
#include "../Core/GameObjectBase.h"
#include "../Core/SingleInstance.h"

struct ImFont;

//The single editor instance (Engine::GetSelf()). Game code reads UI layout info (panel size) through it.
class Engine : public ETG::SingleInstance<Engine>
{
public:
    void LoadFont();
    void Initialize();
    void Update();
    void Draw();
    void DisplayProperties() const;
    static bool IsGameWindowFocused();

    [[nodiscard]] const ETG::Vector2f& GetEngineUISize() const { return windowSize; }

    static bool CurrentGameFocus;
    static bool PreviousGameFocus;

    // Add these static variables to maintain tree node states
    static bool AbsoluteOrientationOpen;
    static bool RelativeOrientationOpen;
    static bool PropertiesOpen;

private:
    void UpdateDetailsPanel();
    friend void ImGuiSetRelativeOrientation(ETG::GameObjectBase* obj);
    friend void ImGuiSetAbsoluteOrientation(ETG::GameObjectBase* obj);
    friend void ImGuiSetRelativeOrientation();

    //By default first time the argument will be scene. After that other objects that has been attached stuffs will be passed.
    //Used recursive depth-first (pre-order) tree traversal
    void DisplayHierarchy(ETG::GameObjectBase* object);

    ImFont* SegoeFont{};
    ETG::Vector2f windowSize;
    ETG::GameObjectBase* SelectedObj = nullptr;

    std::unordered_set<ETG::GameObjectBase*> OwnerObjects;
};
