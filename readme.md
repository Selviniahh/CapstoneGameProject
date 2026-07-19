# Enter the Gungeon Clone

A clone of the game Enter the Gungeon built with SDL3. It aims to replicate the original game, featuring modern C++ design, clean modular code, and a reflection system that lets you easily modify any initialized game object's variables. 

[![Watch the video](https://img.youtube.com/vi/lgvuDcSot1w/0.jpg)](https://youtu.be/lgvuDcSot1w)

## Download the game 
1.  Download at  [release tab](https://github.com/Selviniahh/CapstoneGameProject/releases/download/release/Game.zip). Unzip and open executable file.
2. Also it's possible to download at [itch.io](https://selviniah.itch.io/enter-the-gungeon-clone). Unzip and open the executable file.

# How to build
## Prerequisites
- CMake (3.28+)
- Ninja Build System
- A C++ compiler with C++23 support

No package manager is required. SDL, Dear ImGui, and the required header-only
Boost modules are included as Git submodules under `deps/`.

```
git clone --recurse-submodules https://github.com/Selviniahh/CapstoneGameProject.git
cd CapstoneGameProject
cmake -G Ninja -B build
cmake --build build --parallel --config Release
.\build\bin\ETG.exe
```

### Other Platforms
The build process is similar on Linux and macOS. On Linux, install the usual SDL build dependencies first (X11/Wayland development headers, e.g. `libxcursor-dev libxi-dev libxrandr-dev libxtst-dev libxkbcommon-dev libgl1-mesa-dev libasound2-dev`).

# Dependencies

- SDL3 (window, rendering, and input)
- SDL3_mixer (audio playback and OGG decoding)
- SDL3_image (PNG decoding)
- SDL3_ttf (font rendering)
- Dear ImGui (with the SDL3 + SDL_Renderer3 backends)
- boost-type-index
- boost-describe

# Contributing
Please read the docs folder to understand the project structure.
