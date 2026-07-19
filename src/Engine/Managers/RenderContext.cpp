#include "RenderContext.h"
#include "SpriteBatch.h"

namespace ETG::RenderContext
{
    float DefaultScale = 1;
    std::shared_ptr<ETG::RenderWindow> Window = nullptr;
    ETG::Vector2u ScreenSize;
    ETG::View MainView;

    void Initialize(const std::shared_ptr<ETG::RenderWindow>& window)
    {
        Window = window;
        ScreenSize = window->getSize();
        GlobSpriteBatch.begin();

        //NOTE: Set camera Location and zoom. After enemy, UI, Gun, Hero are handled, better camera and hero locations will be implemented.
        MainView = window->getDefaultView();
        MainView.setCenter(0.f, 0.f);
        MainView.zoom(0.2f);
    }
}
