# Dear ImGui — header-only DEĞİL, SDL3 + SDL_Renderer backend'leriyle birlikte
# bir kütüphane olarak derlenir. Türü BUILD_SHARED_LIBS tarafından belirlenir
# (varsayılan shared). SDL3.cmake'ten SONRA include edilmeli
# (backend dosyaları <SDL3/SDL.h> içerir, SDL3::SDL3 target'ı önce var olmalı).
set(IMGUI_DIR ${DEPS_SOURCE_DIR}/imgui)
      
add_library(imgui
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PUBLIC SDL3::SDL3)

add_library(imgui::imgui ALIAS imgui)
