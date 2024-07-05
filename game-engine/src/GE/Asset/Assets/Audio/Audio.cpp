#include "GE/GEpch.h"

#include "Audio.h"

namespace GE
{
    AudioClip::AudioClip() : Asset()
    {
        p_Type = Asset::Type::AudioClip;
        alGenSources(1, &m_Config.SourceID);
        m_AudioBuffer = CreateRef<AudioBuffer>();
    }

    AudioClip::~AudioClip()
    {
        if (m_AudioBuffer)
        {
            if(m_AudioBuffer->Buffers)
                alDeleteBuffers(m_AudioBuffer->NUM_BUFFERS, m_AudioBuffer->Buffers);
        }

        alDeleteSources(1, &m_Config.SourceID);
    }

    Ref<Asset> AudioClip::GetCopy()
    {
        GE_CORE_WARN("Could not copy AudioClip Asset. Returning nullptr.");
        return nullptr;
    }

    uint64_t AudioClip::GetByteArray(void* buffer, uint64_t bufferSize)
    {
        /*
        * ~ Config
        * * - SourceID : uint32_t
	    * * - Loop : bool 
		* * - Pitch : float
		* * - Gain : float
        * - Position
        * - Velocity
        * ~ Audio Data
        * * - BPS : uint8_t
        * * - Channels : uint8_t
        * * - SampleRate : int32_t
        * * - Format : int32_t
        * * - Size : uint32_t
        * * - Data : uint_8_t*
        */

        return 0;
    }

    void AudioClip::SetSourceValues()
    {
        alSourcei(m_Config.SourceID, AL_LOOPING, m_Config.Loop);
        alSourcef(m_Config.SourceID, AL_PITCH, m_Config.Pitch);
        alSourcef(m_Config.SourceID, AL_GAIN, m_Config.Gain);
        alSource3f(m_Config.SourceID, AL_POSITION, m_Position.x, m_Position.y, m_Position.z);
        alSource3f(m_Config.SourceID, AL_VELOCITY, m_Velocity.x, m_Velocity.y, m_Velocity.z);
    }

    void AudioClip::Play(Config& audioConfig, const glm::vec3& position, const glm::vec3& velocity)
    {
        if (&audioConfig)
        {
            if(&position)
                m_Position = position;
            if (&velocity)
                m_Velocity = velocity;

            m_Config.Gain = audioConfig.Gain;
            m_Config.Pitch = audioConfig.Pitch;
            m_Config.Loop = audioConfig.Loop;
            m_Config.SourceID = audioConfig.SourceID;

            SetSourceValues();
        }

        if (&m_AudioBuffer)
        {
            alSourceQueueBuffers(m_Config.SourceID, m_AudioBuffer->NUM_BUFFERS, m_AudioBuffer->Buffers);
            alSourcePlay(m_Config.SourceID);

            ALint state = AL_PLAYING;
            alGetSourcei(m_Config.SourceID, AL_SOURCE_STATE, &state);

            while (state == AL_PLAYING)
            {
                ALint buffersProcessed = 0;
                alGetSourcei(m_Config.SourceID, AL_BUFFERS_PROCESSED, &buffersProcessed);
                if (buffersProcessed <= 0)
                    return;

                while (buffersProcessed--)
                {
                    AudioBuffer buffer; // Holds unqueued audio buffer
                    alSourceUnqueueBuffers(m_Config.SourceID, 1, buffer.Buffers);

                    ALsizei dataSize = buffer.Size;

                    char* data = new char[dataSize];
                    std::memset(data, 0, dataSize);

                    std::size_t dataSizeToCopy = buffer.Size;
                    if (buffer.Cursor + buffer.Size > sizeof(buffer.Data))
                        dataSizeToCopy = sizeof(buffer.Data) - buffer.Cursor;

                    std::memcpy(&data[0], &buffer.Data[buffer.Cursor], dataSizeToCopy);
                    buffer.Cursor += dataSizeToCopy;

                    if (dataSizeToCopy < buffer.Size)
                    {
                        buffer.Cursor = 0;
                        std::memcpy(&data[dataSizeToCopy], &buffer.Data[buffer.Cursor], buffer.Size - dataSizeToCopy);
                        buffer.Cursor = buffer.Size - dataSizeToCopy;
                    }

                    alBufferData(*buffer.Buffers, buffer.Format, data, buffer.Size, buffer.SampleRate);
                    alSourceQueueBuffers(m_Config.SourceID, 1, buffer.Buffers);

                    delete[] data;
                }

                alGetSourcei(m_Config.SourceID, AL_SOURCE_STATE, &state);
                GE_CORE_TRACE("AudioClip::UpdateBuffers State: {0}", (char*)alGetString(state));
            }
            

        }
    }

    AudioListener::AudioListener()
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

        GE_CORE_INFO("Opened Audio Device: {0}", name);
    }

    AudioListener::~AudioListener()
    {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(m_Context);

        alcCloseDevice(m_Device);
    }
}