#pragma once
#include <string>

struct MIX_Audio;
struct MIX_Track;

namespace ETG
{
    //Decoded sound data (loaded via SDL_mixer), replacement for sf::SoundBuffer
    class SoundBuffer
    {
    public:
        SoundBuffer() = default;
        ~SoundBuffer();

        SoundBuffer(const SoundBuffer&) = delete;
        SoundBuffer& operator=(const SoundBuffer&) = delete;
        SoundBuffer(SoundBuffer&& other) noexcept;
        SoundBuffer& operator=(SoundBuffer&& other) noexcept;

        bool loadFromFile(const std::string& path);

        [[nodiscard]] MIX_Audio* getAudio() const { return m_audio; }

    private:
        MIX_Audio* m_audio = nullptr;
    };

    //One-shot sound playback through an SDL_mixer track, replacement for sf::Sound
    class Sound
    {
    public:
        Sound() = default;
        ~Sound();

        Sound(const Sound&) = delete;
        Sound& operator=(const Sound&) = delete;
        Sound(Sound&& other) noexcept;
        Sound& operator=(Sound&& other) noexcept;

        void setBuffer(const SoundBuffer& buffer);
        void setVolume(float volume); //0..100 like SFML
        void play();
        void stop();

    private:
        bool ensureTrack();

        const SoundBuffer* m_buffer = nullptr;
        MIX_Track* m_track = nullptr;
        float m_volume = 100.f;
    };
}
