# The bgfx renderer

Everything that reaches the screen goes through [bgfx](https://github.com/bkaradzic/bgfx).
SDL3 is still here — it owns the window, the event loop, input, audio and image/font decoding —
but `SDL_Renderer` is gone. That swap is what makes the same renderer run on desktop, in a
browser and on a phone: bgfx picks Direct3D, Vulkan, Metal, OpenGL or WebGL per platform, and the
game never sees the difference.

## The pieces

| File | Job |
| --- | --- |
| `src/Engine/Platform/GraphicsDevice.{h,cpp}` | The device. bgfx init/shutdown, the letterboxed view, the two shader programs, texture upload, every draw call. |
| `src/Engine/Platform/RenderWindow.{h,cpp}` | The SDL window and the 2D camera (`View`). Immediate-mode shapes, lines and text build geometry and hand it to the device. |
| `src/Engine/Managers/SpriteBatch.cpp` | Unchanged in spirit: collect quads, sort by depth, flush one draw call per (texture, shader) run. |
| `src/Engine/Platform/Texture.cpp` | `Image` (an `SDL_Surface`, decoded by SDL3_image) uploaded to a bgfx texture. |
| `src/Engine/Editor/UI/ImGuiBgfxBackend.cpp` | Dear ImGui's renderer backend, replacing `imgui_impl_sdlrenderer3`. Input still comes from `imgui_impl_sdl3`. |
| `src/Engine/Shaders/*.sc` | The shaders themselves. |

## Coordinate spaces

Three of them, from the outside in. Nothing here changed with the port — only who performs the
last step.

```
world  --RenderWindow::worldToScreen (active View)-->  logical  --GraphicsDevice ortho + viewport-->  backbuffer
```

* **world** — where game objects live. The active `View` (centre + size) is the camera.
* **logical** — the fixed 1920x1080 design canvas (`RenderWindow::LogicalSize`). Everything is
  submitted in this space: sprites, the HUD, ImGui. The CPU flattens world to logical, exactly as
  it did before.
* **backbuffer** — real pixels. `GraphicsDevice::GetViewportRect()` letterboxes the logical canvas
  into it and the bars around it are cleared black. This replaces
  `SDL_SetRenderLogicalPresentation`, which is also why `GraphicsDevice::WindowPixelToLogical`
  exists: mouse positions and ImGui clip rectangles have to make the same trip backwards.

Two bgfx views are used: view 0 covers the whole backbuffer and paints the letterbox bars, view 1
is the game viewport with the orthographic projection. View 1 is set to `ViewMode::Sequential`, so
submission order is draw order — that is what keeps `SpriteBatch`'s depth sort meaningful.

## Shaders

Written in bgfx's shader dialect under `src/Engine/Shaders`:

| Shader | What it does |
| --- | --- |
| `vs_sprite.sc` | The only vertex shader. Applies the orthographic projection; positions arrive already flattened into the logical canvas. |
| `fs_sprite.sc` | `texel * vertex colour`. Sprites, text, shapes, ImGui. |
| `fs_sprite_grayscale.sc` | The same, pulled towards its Rec. 601 luminance by `u_effectParams.x`. |

One vertex layout serves everything: position (2 floats), UV (2 floats), packed RGBA8. That is
byte for byte an ImGui `ImDrawVert`, which is why the ImGui backend can hand its vertex buffer
straight to the device with no conversion (there are `static_assert`s guarding it).

Untextured geometry — shapes, bounds, lines — is drawn with `fs_sprite` against a built-in 1x1
white texture, so there is no separate colour program to keep in sync.

### Grayscale on characters

`ETG::ShaderEffect` picks the fragment program per draw. It rides along on
`GameObjectBase::DrawProperties`, so any object can ask for it, but only one does:
`Character` (and therefore `Hero` and every `EnemyBase`) sets `ShaderEffect::Grayscale` in its
constructor. Guns, projectiles, items, VFX and the HUD keep their colour, which is what makes the
effect legible.

```cpp
hero->SetGrayscaleEnabled(false);            // this character only
GraphicsDevice::SetGrayscaleAmount(0.5f);    // global strength, 0..1 (default 1)
```

The shader is deliberately written against the lowest common denominator — no derivatives, no
integer maths, no `textureLod` — so one source compiles for every backend below.

### Compiled shader binaries

A bgfx shader binary is per *backend*, not per host, and it is produced by `shaderc`:

```
Resources/Shaders/glsl/    desktop OpenGL
Resources/Shaders/essl/    WebGL 2 / OpenGL ES 3   (browser, Android)
Resources/Shaders/spirv/   Vulkan
Resources/Shaders/metal/   macOS, iOS
Resources/Shaders/dxil/    Direct3D 12
Resources/Shaders/dxbc/    Direct3D 11             (needs fxc, i.e. a Windows host)
```

`GraphicsDevice` picks the directory at run time from `bgfx::getRendererType()`.

These binaries are **committed**, because a Linux machine cannot produce D3D bytecode and an
Emscripten cross build cannot run `shaderc` at all. A normal desktop build regenerates the ones it
can (`cmake/Shaders.cmake`, driven by the `ETG_COMPILE_SHADERS` option, on by default when not
cross compiling); a cross build just uses what is checked in.

So: **after editing a `.sc` file, build once on Linux/macOS and once on Windows, and commit the
regenerated binaries** — the Windows pass is the only one that can emit `dxbc`.

## Building

### Desktop

Unchanged:

```sh
cmake -G Ninja -B build
cmake --build build --parallel
```

bgfx is built once and cached in `depbuilt/` exactly like SDL. On Linux the usual SDL X11/Wayland
headers are needed, plus GL/EGL ones for bgfx (`libgl1-mesa-dev libegl1-mesa-dev`).

Pass `-DETG_COMPILE_SHADERS=OFF` to skip the shader step and use the committed binaries.

### Browser (Emscripten)

```sh
source /path/to/emsdk/emsdk_env.sh
emcmake cmake -G Ninja -S . -B build-web -DCMAKE_BUILD_TYPE=Release
cmake --build build-web --parallel
```

The output is `build-web/bin/ETG.html` + `.js` + `.wasm` + `.data`. Serve it over HTTP
(`python3 -m http.server`); `file://` will not work because the `.data` package is fetched.

What the web build changes, and why:

* bgfx runs on **WebGL 2** and loads the `essl` shaders. `-sMIN_WEBGL_VERSION=2` and `-sFULL_ES3=1`
  are set in `CMakeLists.txt`.
* The frame loop belongs to the browser: `main.cpp` hands it to `emscripten_set_main_loop` instead
  of blocking, otherwise nothing would ever be painted.
* `Resources/` is packaged into MEMFS with `--preload-file`, so `AssetManager` keeps working with
  plain paths.
* Dependencies are built statically into their own cache (`depbuilt/Emscripten-static/`), and
  `BUILD_SHARED_LIBS` is forced off.
* bgfx is single threaded here (it forces that on Emscripten, and the device asks for it anyway by
  calling `bgfx::renderFrame()` before `bgfx::init()`).

### Mobile

The renderer itself needs nothing platform-specific beyond what
`GraphicsDevice::BuildPlatformData` already does:

* **Android** — bgfx gets the `ANativeWindow` from SDL and renders through OpenGL ES 3 or Vulkan;
  it uses the `essl`/`spirv` binaries. Build with the NDK toolchain file
  (`-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake`) inside SDL3's
  Android project, and package `Resources/` under the APK's `assets/` — `AssetManager` already
  resolves relative paths through SDL's IO layer there.
* **iOS** — the window is created with `SDL_WINDOW_METAL` and bgfx is handed the `CAMetalLayer`
  from `SDL_Metal_CreateView`, so it renders through Metal with the `metal` binaries.

Touch input is a separate matter and is not part of this: the game still reads mouse and keyboard.

## Things worth knowing

* **Draw order.** The scene view is sequential, so immediate draws (`Window->draw(circle)`) land
  in the order they are called, before the sprite batch that is flushed after them — the same
  layering the SDL renderer produced.
* **Transient buffers.** Every draw allocates from bgfx's transient vertex/index buffers. If a
  frame ever exceeds them the geometry is dropped rather than asserting, and `SpriteBatch` splits
  runs at 16384 quads to stay inside 16-bit indices.
* **Sampling** is baked into the texture: sprites are point sampled (pixel art stays crisp when
  the view zooms), text and the ImGui atlas are linear.
