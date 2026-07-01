#include "Audio.h"
#include <cstdlib>
#include <SDL3/SDL.h>

//stb_vorbis.c doubles as its own header when STB_VORBIS_HEADER_ONLY is defined.
//The implementation is compiled once in StbImpl.cpp.
#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>
#undef STB_VORBIS_HEADER_ONLY

namespace ETG
{
    namespace
    {
        //Single shared playback device, opened lazily on first use.
        //Returns 0 when no audio device is available (e.g. headless); sounds become no-ops.
        SDL_AudioDeviceID EnsureAudioDevice()
        {
            static SDL_AudioDeviceID device = []() -> SDL_AudioDeviceID
            {
                if (!SDL_WasInit(SDL_INIT_AUDIO))
                {
                    if (!SDL_InitSubSystem(SDL_INIT_AUDIO))
                        return 0;
                }
                return SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
            }();
            return device;
        }
    }

    //------------------------------------ SoundBuffer ------------------------------------
    bool SoundBuffer::loadFromFile(const std::string& path)
    {
        short* output = nullptr;
        int channels = 0, sampleRate = 0;
        const int sampleCount = stb_vorbis_decode_filename(path.c_str(), &channels, &sampleRate, &output);
        if (sampleCount <= 0 || !output) return false;

        m_samples.assign(output, output + static_cast<size_t>(sampleCount) * channels);
        m_channels = channels;
        m_sampleRate = sampleRate;
        free(output);
        return true;
    }

    //------------------------------------ Sound ------------------------------------
    Sound::~Sound()
    {
        if (m_stream) SDL_DestroyAudioStream(m_stream);
    }

    Sound::Sound(Sound&& other) noexcept : m_buffer(other.m_buffer), m_stream(other.m_stream), m_volume(other.m_volume)
    {
        other.m_stream = nullptr;
        other.m_buffer = nullptr;
    }

    Sound& Sound::operator=(Sound&& other) noexcept
    {
        if (this == &other) return *this;
        if (m_stream) SDL_DestroyAudioStream(m_stream);
        m_buffer = other.m_buffer;
        m_stream = other.m_stream;
        m_volume = other.m_volume;
        other.m_stream = nullptr;
        other.m_buffer = nullptr;
        return *this;
    }

    void Sound::setBuffer(const SoundBuffer& buffer)
    {
        m_buffer = &buffer;
        if (m_stream)
        {
            SDL_DestroyAudioStream(m_stream);
            m_stream = nullptr;
        }
    }

    bool Sound::ensureStream()
    {
        if (m_stream) return true;
        if (!m_buffer || m_buffer->getSamples().empty()) return false;

        const SDL_AudioDeviceID device = EnsureAudioDevice();
        if (device == 0) return false;

        const SDL_AudioSpec srcSpec{SDL_AUDIO_S16, m_buffer->getChannelCount(), m_buffer->getSampleRate()};
        m_stream = SDL_CreateAudioStream(&srcSpec, nullptr);
        if (!m_stream) return false;

        if (!SDL_BindAudioStream(device, m_stream))
        {
            SDL_DestroyAudioStream(m_stream);
            m_stream = nullptr;
            return false;
        }

        SDL_SetAudioStreamGain(m_stream, m_volume / 100.f);
        return true;
    }

    void Sound::setVolume(const float volume)
    {
        m_volume = volume;
        if (m_stream) SDL_SetAudioStreamGain(m_stream, m_volume / 100.f);
    }

    void Sound::play()
    {
        if (!ensureStream()) return;

        const auto& samples = m_buffer->getSamples();
        SDL_ClearAudioStream(m_stream); //Restart from the beginning like sf::Sound::play
        SDL_PutAudioStreamData(m_stream, samples.data(), static_cast<int>(samples.size() * sizeof(std::int16_t)));
        SDL_FlushAudioStream(m_stream);
    }

    void Sound::stop()
    {
        if (m_stream) SDL_ClearAudioStream(m_stream);
    }
}
