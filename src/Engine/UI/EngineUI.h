#pragma once

namespace ETG
{
    class GameObject;

    class EngineUI
    {
        friend void ImGuiSetRelativeOrientation(GameObject* obj);
        friend void ImGuiSetAbsoluteOrientation(GameObject* obj);
    };
}


