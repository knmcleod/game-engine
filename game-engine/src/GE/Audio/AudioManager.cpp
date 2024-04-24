#include "GE/GEpch.h"

#include "AudioManager.h"

#include "AudioDevice.h"

namespace GE
{
    static std::int32_t convert_to_int(char* buffer, std::size_t len)
    {
        std::int32_t a = 0;
        std::memcpy(&a, buffer, len);
        return a;
    }

    AudioManager::AudioManager()
    {
        m_AudioDevice = CreateRef<AudioDevice>();
        m_AudioSources = std::vector<Ref<AudioSource>>();
        m_AudioBuffers = std::vector<AudioBuffer>();
        m_LongAudioBuffers = std::vector<LongAudioBuffer>();
    }

    AudioManager::~AudioManager()
    {
        for (const auto& source : m_AudioSources)
        {
            alDeleteSources(1, &source->GetSource());
        }
        m_AudioSources.clear();

        for (const auto& buffer : m_AudioBuffers)
        {
            alDeleteBuffers(1, &buffer.Buffer);
        }
        m_AudioBuffers.clear();

        for (const auto& buffer : m_LongAudioBuffers)
        {
            alDeleteBuffers(buffer.NUM_BUFFERS, buffer.Buffers);
        }
        m_LongAudioBuffers.clear();
    }

    bool AudioManager::AddSound(Ref<AudioSource> source, const std::filesystem::path& filePath)
    {
        AudioBuffer buffer;
        buffer.Data = LoadWav(filePath, buffer.Channels, buffer.SampleRate, buffer.BPS, buffer.Size);
        buffer.FilePath = filePath;

        if (buffer.Channels == 1 && buffer.BPS == 8)
            buffer.Format = AL_FORMAT_MONO8;
        else if (buffer.Channels == 1 && buffer.BPS == 16)
            buffer.Format = AL_FORMAT_MONO16;
        else if (buffer.Channels == 2 && buffer.BPS == 8)
            buffer.Format = AL_FORMAT_STEREO8;
        else if (buffer.Channels == 2 && buffer.BPS == 16)
            buffer.Format = AL_FORMAT_STEREO16;
        else
        {
            GE_CORE_ERROR("Unrecognized wave format.\nChannels {0}\nBPS {1}\n", buffer.Channels, buffer.BPS);
            return 0;
        }

        source->AddSound(buffer);

        return true;
    }

    bool AudioManager::AddSound(const std::filesystem::path& filePath)
    {
        Ref<AudioSource> source = CreateRef<AudioSource>();
 
        if (AddSound(source, filePath))
        {
            m_AudioSources.push_back(source);
            return true;
        }
        return false;

    }

    bool AudioManager::AddMusic(Ref<AudioSource> source, const std::filesystem::path& filePath)
    {
        LongAudioBuffer& buffer = LongAudioBuffer();

        if (LoadWav(filePath, buffer))
        {
            buffer.FilePath = filePath;

            if (buffer.Channels == 1 && buffer.BPS == 8)
                buffer.Format = AL_FORMAT_MONO8;
            else if (buffer.Channels == 1 && buffer.BPS == 16)
                buffer.Format = AL_FORMAT_MONO16;
            else if (buffer.Channels == 2 && buffer.BPS == 8)
                buffer.Format = AL_FORMAT_STEREO8;
            else if (buffer.Channels == 2 && buffer.BPS == 16)
                buffer.Format = AL_FORMAT_STEREO16;
            else
            {
                GE_CORE_ERROR("Unrecognized wave format.\nChannels {0}\nBPS {1}\n", buffer.Channels, buffer.BPS);
                return 0;
            }

            source->AddMusic(buffer);

            m_LongAudioBuffers.push_back(buffer);
            return true;
        }
        return false;
    }

    bool AudioManager::AddMusic(const std::filesystem::path& filePath)
    {
        Ref<AudioSource> source = CreateRef<AudioSource>();

        if (AddMusic(source, filePath))
        {
            m_AudioSources.push_back(source);
            return true;
        }

        return false;
    }

    bool AudioManager::RemoveSource(Ref<AudioSource> source)
    {
        auto it = m_AudioSources.begin();
        while (it != m_AudioSources.end())
        {
            if (it->get()->GetHandle() == source->GetHandle())
            {
                m_AudioSources.erase(it);
                return true;
            }
            
            ++it;
        }
        return false;
    }

    void AudioManager::PlaySounds()
    {
        auto it = m_AudioSources.begin();
        while (it != m_AudioSources.end())
        {
            for each (AudioBuffer buffer in m_AudioBuffers)
            {
                it->get()->Play(buffer);
            }
            ++it;
        }
    }

    void AudioManager::PlayMusic()
    {
        auto it = m_AudioSources.begin();
        while (it != m_AudioSources.end())
        {
            for each (LongAudioBuffer buffer in m_LongAudioBuffers)
            {
                it->get()->Play(buffer);
            }
            ++it;
        }
    }

    bool AudioManager::LoadWavFile(std::ifstream& file,
        std::uint8_t& channels,
        std::int32_t& sampleRate,
        std::uint8_t& bitsPerSample,
        ALsizei& size)
    {
        char buffer[4];
        if (!file.is_open())
            return false;

        // the RIFF
        if (!file.read(buffer, 4))
        {
            GE_CORE_ERROR("Could not read RIFF while loading Wav file.");
            return false;
        }
        if (std::strncmp(buffer, "RIFF", 4) != 0)
        {
            GE_CORE_ERROR("File is not a valid WAVE file (header doesn't begin with RIFF)");
            return false;
        }

        // the size of the file
        if (!file.read(buffer, 4))
        {
            GE_CORE_ERROR("Could not read size of Wav file.");
            return false;
        }

        // the WAVE
        if (!file.read(buffer, 4))
        {
            GE_CORE_ERROR("Could not read WAVE");
            return false;
        }
        if (std::strncmp(buffer, "WAVE", 4) != 0)
        {
            GE_CORE_ERROR("File is not a valid WAVE file (header doesn't contain WAVE)");
            return false;
        }

        // "fmt/0"
        if (!file.read(buffer, 4))
        {
            GE_CORE_ERROR("Could not read fmt of Wav file.");
            return false;
        }

        // this is always 16, the size of the fmt data chunk
        if (!file.read(buffer, 4))
        {
            GE_CORE_ERROR("Could not read the size of the fmt data chunk. Should be 16.");
            return false;
        }

        // PCM should be 1?
        if (!file.read(buffer, 2))
        {
            GE_CORE_ERROR("Could not read PCM. Should be 1.");
            return false;
        }

        // the number of Channels
        if (!file.read(buffer, 2))
        {
            GE_CORE_ERROR("Could not read number of Channels.");
            return false;
        }
        channels = convert_to_int(buffer, 2);

        // sample rate
        if (!file.read(buffer, 4))
        {
            GE_CORE_ERROR("Could not read sample rate.");
            return false;
        }
        sampleRate = convert_to_int(buffer, 4);

        // (SampleRate * BPS * Channels) / 8
        if (!file.read(buffer, 4))
        {
            GE_CORE_ERROR("Could not read (SampleRate * BPS * Channels) / 8");
            return false;
        }

        // ?? dafaq
        if (!file.read(buffer, 2))
        {
            GE_CORE_ERROR("Could not read dafaq?");
            return false;
        }

        // BPS
        if (!file.read(buffer, 2))
        {
            GE_CORE_ERROR("Could not read bits per sample.");
            return false;
        }
        bitsPerSample = convert_to_int(buffer, 2);

        // data chunk header "data"
        if (!file.read(buffer, 4))
        {
            GE_CORE_ERROR("Could not read data chunk header.");
            return false;
        }
        if (std::strncmp(buffer, "data", 4) != 0)
        {
            GE_CORE_ERROR("File is not a valid WAVE file (doesn't have 'data' tag).");
            return false;
        }

        // size of data
        if (!file.read(buffer, 4))
        {
            GE_CORE_ERROR("Could not read data size.");
            return false;
        }
        size = convert_to_int(buffer, 4);

        /* cannot be at the end of file */
        if (file.eof())
        {
            GE_CORE_ERROR("Reached EOF.");
            return false;
        }
        if (file.fail())
        {
            GE_CORE_ERROR("Fail state set on the file.");
            return false;
        }

        return true;
    }

    char* AudioManager::LoadWav(const std::filesystem::path& filePath,
        std::uint8_t& channels,
        std::int32_t& sampleRate,
        std::uint8_t& bitsPerSample,
        ALsizei& size)
    {
        std::ifstream stream(filePath, std::ios::binary);
        if (!stream.is_open())
        {
            GE_CORE_ERROR("Could not open WAV file {0}", filePath.string().c_str());
            return nullptr;
        }
        if (!LoadWavFile(stream, channels, sampleRate, bitsPerSample, size))
        {
            GE_CORE_ERROR("Could not load WAV file {0}", filePath.string().c_str());
            return nullptr;
        }

        char* data = new char[size];

        stream.read(data, size);
        stream.close();

        return data;
    }

    bool AudioManager::LoadWav(const std::filesystem::path& filePath, LongAudioBuffer& buffer)
    {
        std::ifstream stream(filePath, std::ios::binary);
        if (!stream.is_open())
        {
            GE_CORE_ERROR("Could not open WAV file {0}", filePath.string().c_str());
            return false;
        }
        ALsizei size = buffer.BUFFER_SIZE;
        for (int i = 0; i < buffer.NUM_BUFFERS; i++)
        {
            if (!LoadWavFile(stream, buffer.Channels, buffer.SampleRate, buffer.BPS, size))
            {
                GE_CORE_ERROR("Could not load WAV file {0}", filePath.string().c_str());
                continue;
            }

            char* soundData = new char[size];
            stream.read(soundData, size);
            buffer.Data[i] = soundData;
        }
        stream.close();
        return true;
    }
}