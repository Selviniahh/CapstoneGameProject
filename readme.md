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

No package manager is required: SDL3 and Boost are downloaded automatically by CMake (FetchContent), while Dear ImGui and stb are vendored under `external/`.

```
git clone https://github.com/Selviniahh/CapstoneGameProject.git
cd CapstoneGameProject
cmake -G Ninja -B build
cmake --build build --config Release
.\build\bin\ETG.exe
```

### Other Platforms
The build process is similar on Linux and macOS. On Linux, install the usual SDL build dependencies first (X11/Wayland development headers, e.g. `libxcursor-dev libxi-dev libxrandr-dev libxtst-dev libxkbcommon-dev libgl1-mesa-dev libasound2-dev`).

# Dependencies

- SDL3 (window, rendering, input, audio)
- Dear ImGui (with the SDL3 + SDL_Renderer3 backends)
- stb (stb_image, stb_truetype, stb_vorbis for asset decoding)
- boost-type-index
- boost-describe
- boost-mpl

# Contributing
Please read the docs folder to understand the project structure.
