#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct SDL_AudioStream;

namespace ETG
{
    //Decoded PCM sound data (OGG Vorbis via stb_vorbis), replacement for sf::SoundBuffer
    class SoundBuffer
    {
    public:
        SoundBuffer() = default;

        bool loadFromFile(const std::string& path);

        [[nodiscard]] const std::vector<std::int16_t>& getSamples() const { return m_samples; }
        [[nodiscard]] int getChannelCount() const { return m_channels; }
        [[nodiscard]] int getSampleRate() const { return m_sampleRate; }

    private:
        std::vector<std::int16_t> m_samples;
        int m_channels = 0;
        int m_sampleRate = 0;
    };

    //One-shot sound playback through an SDL3 audio stream, replacement for sf::Sound
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
        bool ensureStream();

        const SoundBuffer* m_buffer = nullptr;
        SDL_AudioStream* m_stream = nullptr;
        float m_volume = 100.f;
    };
}
