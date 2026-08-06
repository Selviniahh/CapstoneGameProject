# Enter the Gungeon Clone

A clone of the game Enter the Gungeon built with SDL3 and bgfx. It aims to replicate the original game, featuring modern C++ design, clean modular code, and a reflection system that lets you easily modify any initialized game object's variables.

SDL3 owns the window, input, audio and asset decoding; everything that reaches the screen is drawn by bgfx, so the same renderer and shaders run on desktop, in the browser (WebGL 2 via Emscripten) and on mobile. See [docs/BgfxRenderer.md](docs/BgfxRenderer.md).

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
The build process is similar on Linux and macOS. On Linux, install the usual SDL build dependencies first, plus the GL/EGL headers bgfx needs (X11/Wayland development headers, e.g. `libxcursor-dev libxi-dev libxrandr-dev libxtst-dev libxkbcommon-dev libgl1-mesa-dev libegl1-mesa-dev libasound2-dev`).

### Browser (WebAssembly)
With the [emsdk](https://emscripten.org/docs/getting_started/downloads.html) active:

```
emcmake cmake -G Ninja -S . -B build-web -DCMAKE_BUILD_TYPE=Release
cmake --build build-web --parallel
```

This produces `build-web/bin/ETG.html`, which has to be served over HTTP (`python3 -m http.server`) rather than opened from disk. Mobile builds and the details of the renderer are covered in [docs/BgfxRenderer.md](docs/BgfxRenderer.md).

# Tests

Two suites live under [`Test/`](Test/README.md), each producing its own executable:

- **`ETGUnitTests`** — GoogleTest, console only. Pure logic: the angle → direction table, `StatModifier`, the shared maths helpers.
- **`ETGInteractiveTests`** — the real engine booted with an *empty* world. One gameplay test is loaded at a time and builds its own scene (its own hero, enemies, guns); its assertions sit at `PENDING` in an ImGui panel and flip to `PASSED`/`FAILED` as you play. This is where a mechanic gets exercised without touching the game's own level.

```
cmake -G Ninja -B build          # both test targets are built by default
./build/bin/ETGUnitTests         # or: ctest --test-dir build
./build/bin/ETGInteractiveTests
```

Either suite can also be made a gate on the game's own build, so `ETG` will not start until it passes:

```
cmake -G Ninja -B build -DETG_RUN_UNIT_TESTS_BEFORE_GAME=ON -DETG_RUN_INTERACTIVE_TESTS_BEFORE_GAME=ON
```

Both default to `OFF`; [`Test/README.md`](Test/README.md) covers the full option matrix and how to write a new test of either kind.

# Dependencies

- SDL3 (window, input, and event loop)
- bgfx (rendering: Direct3D / Vulkan / Metal / OpenGL / WebGL)
- SDL3_mixer (audio playback and OGG decoding)
- SDL3_image (PNG decoding)
- SDL3_ttf (font rendering)
- Dear ImGui (SDL3 platform backend; the renderer backend is the project's own, on bgfx)
- boost-type-index
- boost-describe
- GoogleTest (unit tests only; a missing `deps/googletest` submodule just disables them)

# Contributing
Please read the docs folder to understand the project structure.
