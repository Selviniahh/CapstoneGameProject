#include "Audio.h"
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

namespace ETG
{
    namespace
    {
        //Single shared mixer, opened lazily on first use.
        //Returns nullptr when no audio device is available (e.g. headless); sounds become no-ops.
        MIX_Mixer* EnsureMixer()
        {
            static MIX_Mixer* mixer = nullptr;
            static bool initialized = false;

            if (!initialized)
            {
                if (MIX_Init()) mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
                initialized = true;
            }

            return mixer;
        }
    }

    //------------------------------------ SoundBuffer ------------------------------------
    SoundBuffer::~SoundBuffer()
    {
        if (m_audio) MIX_DestroyAudio(m_audio);
    }

    SoundBuffer::SoundBuffer(SoundBuffer&& other) noexcept : m_audio(other.m_audio)
    {
        other.m_audio = nullptr;
    }

    SoundBuffer& SoundBuffer::operator=(SoundBuffer&& other) noexcept
    {
        if (this == &other) return *this;
        if (m_audio) MIX_DestroyAudio(m_audio);
        m_audio = other.m_audio;
        other.m_audio = nullptr;
        return *this;
    }

    bool SoundBuffer::loadFromFile(const std::string& path)
    {
        MIX_Mixer* mixer = EnsureMixer();
        if (!mixer) return false;

        m_audio = MIX_LoadAudio(mixer, path.c_str(), true /*predecode*/);
        return m_audio != nullptr;
    }

    //------------------------------------ Sound ------------------------------------
    Sound::~Sound()
    {
        if (m_track) MIX_DestroyTrack(m_track);
    }

    Sound::Sound(Sound&& other) noexcept : m_buffer(other.m_buffer), m_track(other.m_track), m_volume(other.m_volume)
    {
        other.m_track = nullptr;
        other.m_buffer = nullptr;
    }

    Sound& Sound::operator=(Sound&& other) noexcept
    {
        if (this == &other) return *this;
        if (m_track) MIX_DestroyTrack(m_track);
        m_buffer = other.m_buffer;
        m_track = other.m_track;
        m_volume = other.m_volume;
        other.m_track = nullptr;
        other.m_buffer = nullptr;
        return *this;
    }

    void Sound::setBuffer(const SoundBuffer& buffer)
    {
        m_buffer = &buffer;
        if (m_track) MIX_SetTrackAudio(m_track, m_buffer->getAudio());
    }

    bool Sound::ensureTrack()
    {
        if (m_track) return true;
        if (!m_buffer || !m_buffer->getAudio()) return false;

        MIX_Mixer* mixer = EnsureMixer();
        if (!mixer) return false;

        m_track = MIX_CreateTrack(mixer);
        if (!m_track) return false;

        MIX_SetTrackAudio(m_track, m_buffer->getAudio());
        MIX_SetTrackGain(m_track, m_volume / 100.f);
        return true;
    }

    void Sound::setVolume(const float volume)
    {
        m_volume = volume;
        if (m_track) MIX_SetTrackGain(m_track, m_volume / 100.f);
    }

    void Sound::play()
    {
        if (!ensureTrack()) return;

        //options=0: play once from the start, restarting from the beginning if already
        //playing, like sf::Sound::play
        MIX_PlayTrack(m_track, 0);
    }

    void Sound::stop()
    {
        if (m_track) MIX_StopTrack(m_track, 0);
    }
}
