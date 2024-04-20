#include "GE/GEpch.h"

#include "Audio.h"
 
namespace GE
{
    AudioDevice::AudioDevice()
    {
        m_Device = alcOpenDevice(nullptr); // Returns default device
        if (!m_Device)
        {
            GE_CORE_ERROR("Could not open audio device!");
            return;
        }

        m_Context = alcCreateContext(m_Device, nullptr); // Returns default context
        if (!m_Device)
        {
            GE_CORE_ERROR("Could not create audio device context!");
            return;
        }

        if (!alcMakeContextCurrent(m_Context))
        {
            GE_CORE_ERROR("Could not make audio device context current!");
            return;
        }

        const ALCchar* name = nullptr;
        if (alcIsExtensionPresent(m_Device, "ALC_ENUMERATE_ALL_EXT"))
            name = alcGetString(m_Device, ALC_ALL_DEVICES_SPECIFIER);

        if (!name || alcGetError(m_Device) != ALC_NO_ERROR)
            name = alcGetString(m_Device, ALC_DEVICE_SPECIFIER);

        GE_CORE_INFO("Opened AudioManager Device: {0}", name);
    }

    AudioDevice::~AudioDevice()
    {
    
    }

    AudioManager::AudioManager()
    {
        m_AudioBuffers = std::vector<AudioBuffer>();

        m_AudioDevice = CreateRef<AudioDevice>();
        m_AudioSource = CreateRef<AudioSource>();
    }

    AudioManager::~AudioManager()
    {
        alDeleteBuffers(m_AudioBuffers.size(), &m_AudioBuffers.data()->m_AudioBuffer);
        m_AudioBuffers.clear();
    }

    static std::int32_t convert_to_int(char* buffer, std::size_t len)
    {
        std::int32_t a = 0;
        std::memcpy(&a, buffer, len);
        return a;
    }

    void AudioManager::PlaySounds()
    {
        auto it = m_AudioBuffers.begin();
        while (it != m_AudioBuffers.end())
        {
            m_AudioSource->Play(*it);
            ++it;
        }
    }

    bool AudioManager::LoadWavFile(std::ifstream& file, AudioBuffer& audioBuffer)
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
        audioBuffer.Channels = convert_to_int(buffer, 2);

        // sample rate
        if (!file.read(buffer, 4))
        {
            GE_CORE_ERROR("Could not read sample rate.");
            return false;
        }
        audioBuffer.SampleRate = convert_to_int(buffer, 4);

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
        audioBuffer.BPS = convert_to_int(buffer, 2);

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
        audioBuffer.Size = convert_to_int(buffer, 4);

        /* cannot be at the end of file */
        if (file.eof())
        {
            GE_CORE_ERROR("Reached EOF on the file.");
            return false;
        }
        if (file.fail())
        {
            GE_CORE_ERROR("Fail state set on the file.");
            return false;
        }

        return true;
    }

    // Move to AudioBuffer?
    bool AudioManager::AddSound(const std::filesystem::path& filePath)
    {
        AudioBuffer buffer;

        std::ifstream stream(filePath, std::ios::binary);
        if (!stream.is_open())
        {
            GE_CORE_ERROR("Could not open {0}", filePath.string().c_str());
            return 0;
        }
        if (!LoadWavFile(stream, buffer))
        {
            GE_CORE_ERROR("Could not load wav file {0}", filePath.filename().string().c_str());
            return 0;
        }

       buffer.Data = new char[buffer.Size];
        stream.read(buffer.Data, buffer.Size);
        stream.close();

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
            GE_CORE_ERROR("Unrecognised wave format.\nChannels {0}\nBPS {1}\n", buffer.Channels, buffer.BPS);
            return 0;
        }

        // Create AL Buffer
        alGenBuffers(1, &buffer.m_AudioBuffer);
        alBufferData(buffer.m_AudioBuffer, buffer.Format, buffer.Data, buffer.Size, buffer.SampleRate);

        ALenum error = alGetError();
        if (error != AL_NO_ERROR)
        {
            GE_CORE_ERROR("OpenAL Error: {0}", (char*)alGetString(error));
            if (buffer.m_AudioBuffer && alIsBuffer(buffer.m_AudioBuffer))
                alDeleteBuffers(1, &buffer.m_AudioBuffer);
            return 0;
        }

        m_AudioBuffers.push_back(buffer);
        return true;

    }

    bool AudioManager::RemoveSound(AudioBuffer audioBuffer)
    {
        auto it = m_AudioBuffers.begin();
        while (it != m_AudioBuffers.end())
        {
            if (it->m_AudioBuffer == audioBuffer.m_AudioBuffer)
            {
                alDeleteBuffers(1, &it->m_AudioBuffer);
                it = m_AudioBuffers.erase(it);
                return true;
            }
            else
            {
                ++it; // Next sound
            }
        }

        return false;
    }

    AudioSource::AudioSource()
    {
        alGenSources(1, &m_AudioSource);
        alSourcef(m_AudioSource, AL_PITCH, m_Pitch);
        alSourcef(m_AudioSource, AL_GAIN, m_Gain);
        alSource3f(m_AudioSource, AL_POSITION, m_Position.x, m_Position.y, m_Position.z);
        alSource3f(m_AudioSource, AL_VELOCITY, m_Vecloity.x, m_Vecloity.y, m_Vecloity.z);
        alSourcei(m_AudioSource, AL_LOOPING, m_Loop);
        alSourcei(m_AudioSource, AL_BUFFER, 0);
    }

    AudioSource::~AudioSource()
    {
        alDeleteSources(1, &m_AudioSource);
    }

    void AudioSource::Play(AudioBuffer audioBuffer)
    {
        alSourcei(m_AudioSource, AL_BUFFER, audioBuffer.m_AudioBuffer);
        alSourcePlay(m_AudioSource);
    }
}