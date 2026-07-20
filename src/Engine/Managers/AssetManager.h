#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace ETG
{
    class Texture;

    //Resolves asset paths at runtime so the game is portable across machines and platforms.
    //Desktop: <executable dir>/Resources (CMake copies the Resources folder next to the binary after every build).
    //Android: returns relative "Resources/..." paths; SDL's IO layer (used by SDL_image/mixer/ttf) reads those
    //directly from the APK's assets, so no extraction step is needed.
    class AssetManager
    {
    public:
        //Called lazily by Resolve(), but call it explicitly once at startup (after SDL init) to fail early.
        static void Initialize();

        //Turn a path relative to the Resources root (e.g. "Sounds/Pickup1.ogg") into a path loadable
        //by the SDL family of loaders on the current platform.
        static std::string Resolve(const std::filesystem::path& relativePath);

        //Read a whole asset into memory through SDL's IO layer (works from APK assets on Android).
        //Returns an empty vector on failure. Use for libraries that can't take a file path (e.g. ImGui fonts).
        static std::vector<unsigned char> LoadBytes(const std::filesystem::path& relativePath);

        //Load a texture through a process-wide cache. Every caller asking for the same file shares one
        //Texture instance, so the sprite batch can merge their draws (it batches by texture pointer) and
        //repeated spawns cause no disk IO. Accepts raw relative paths and paths already passed through
        //Resolve(). Throws if the file can't be loaded.
        static std::shared_ptr<Texture> LoadTexture(const std::filesystem::path& relativePath);

    private:
        static std::filesystem::path ResourceRoot;
        static bool Initialized;
    };
}
