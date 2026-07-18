//hello world
#include "AssetManager.h"
#include <SDL3/SDL.h>

namespace ETG
{
    std::filesystem::path AssetManager::ResourceRoot;
    bool AssetManager::Initialized = false;

    void AssetManager::Initialize()
    {
#ifdef SDL_PLATFORM_ANDROID
        //A relative path makes SDL_IOFromFile read from the APK's assets/ directory,
        //so the Resources folder just needs to be packaged under assets/Resources.
        ResourceRoot = "Resources";
#else
        //SDL owns the returned string in SDL3; it must not be freed
        if (const char* base = SDL_GetBasePath())
            ResourceRoot = std::filesystem::path(base) / "Resources";
        else
            ResourceRoot = "Resources";

        //Dev fallback: when running from a build layout where Resources wasn't copied yet,
        //walk up from the executable towards the project root and use the first match.
        if (!std::filesystem::exists(ResourceRoot))
        {
            std::filesystem::path dir = ResourceRoot.parent_path();
            while (dir.has_parent_path() && dir != dir.parent_path())
            {
                dir = dir.parent_path();
                std::filesystem::path candidate = dir / "Resources";
                if (std::filesystem::exists(candidate))
                {
                    ResourceRoot = candidate;
                    break;
                }
            }
        }
#endif
        Initialized = true;
    }

    std::string AssetManager::Resolve(const std::filesystem::path& relativePath)
    {
        if (!Initialized) Initialize();
        return (ResourceRoot / relativePath).generic_string();
    }

    std::vector<unsigned char> AssetManager::LoadBytes(const std::filesystem::path& relativePath)
    {
        std::size_t size = 0;
        void* data = SDL_LoadFile(Resolve(relativePath).c_str(), &size);
        if (!data) return {};

        unsigned char* bytes = static_cast<unsigned char*>(data);
        std::vector<unsigned char> result(bytes, bytes + size);
        SDL_free(data);
        return result;
    }
}
