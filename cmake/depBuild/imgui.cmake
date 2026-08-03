# Dear ImGui — header-only DEĞİL, SDL3 platform backend'iyle birlikte bir kütüphane
# olarak derlenir. Türü BUILD_SHARED_LIBS tarafından belirlenir (varsayılan shared).
# SDL3.cmake'ten SONRA include edilmeli (backend dosyası <SDL3/SDL.h> içerir,
# SDL3::SDL3 target'ı önce var olmalı).
#
# NOT: renderer backend'i burada yok. SDL_Renderer yerine bgfx kullanıldığı için çizim
# tarafını proje kendi backend'iyle karşılıyor: src/Engine/Editor/UI/ImGuiBgfxBackend.cpp
set(IMGUI_DIR ${DEPS_SOURCE_DIR}/imgui)
      
add_library(imgui
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
)

target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PUBLIC SDL3::SDL3)

add_library(imgui::imgui ALIAS imgui)
