#include "GE/GEpch.h"

#include "Audio.h"
 
namespace GE
{
    AudioSource::AudioSource()
    {
        m_AudioBuffers = std::vector<AudioBuffer>();
        m_LongAudioBuffers = std::vector<LongAudioBuffer>();

        alGenSources(1, &m_Source);
        alSourcef(m_Source, AL_PITCH, m_Pitch);
        alSourcef(m_Source, AL_GAIN, m_Gain);
        alSource3f(m_Source, AL_POSITION, m_Position.x, m_Position.y, m_Position.z);
        alSource3f(m_Source, AL_VELOCITY, m_Velocity.x, m_Velocity.y, m_Velocity.z);
        alSourcei(m_Source, AL_LOOPING, m_Loop);
        alSourcei(m_Source, AL_BUFFER, 0);
    }

    AudioSource::~AudioSource()
    {
        alDeleteBuffers(m_AudioBuffers.size(), &m_AudioBuffers.data()->Buffer);
        m_AudioBuffers.clear();

        alDeleteBuffers(m_LongAudioBuffers.data()->NUM_BUFFERS, m_LongAudioBuffers.data()->Buffers);
        m_LongAudioBuffers.clear();
    }

    bool AudioSource::AddSound(AudioBuffer& buffer)
    {
        // Create AL Buffer
        alGenBuffers(1, (ALuint*)&buffer.Buffer);
        alBufferData(buffer.Buffer, buffer.Format, buffer.Data, buffer.Size, buffer.SampleRate);

        ALenum error = alGetError();
        if (error != AL_NO_ERROR)
        {
            GE_CORE_ERROR("OpenAL Error: {0}", (char*)alGetString(error));
            if (buffer.Buffer && alIsBuffer(buffer.Buffer))
                alDeleteBuffers(1, &buffer.Buffer);
            return false;
        }

        m_AudioBuffers.push_back(buffer);
        return true;
    }

    bool AudioSource::RemoveSound(const AudioBuffer& buffer)
    {
        auto it = m_AudioBuffers.begin();
        while (it != m_AudioBuffers.end())
        {
            if (it->Buffer == buffer.Buffer)
            {
                alDeleteBuffers(1, &it->Buffer);
                it = m_AudioBuffers.erase(it);
                return true;
            }
            else
            {
                ++it; // Next Audio Buffer
            }
        }

        return false;
    }

    bool AudioSource::AddMusic(LongAudioBuffer& buffer)
    {
        alGenBuffers(buffer.NUM_BUFFERS, &buffer.Buffers[0]);

        for (std::size_t i = 0; i < buffer.NUM_BUFFERS; ++i)
        {
            alBufferData(buffer.Buffers[i], buffer.Format, &buffer.Data[i], buffer.BUFFER_SIZE, buffer.SampleRate);

            ALenum error = alGetError();
            if (error != AL_NO_ERROR)
            {
                GE_CORE_ERROR("OpenAL Error: {0}", (char*)alGetString(error));
                if (buffer.Buffers[i] && alIsBuffer(buffer.Buffers[i]))
                    alDeleteBuffers(1, &buffer.Buffers[i]);
                return false;
            }
        }

        m_LongAudioBuffers.push_back(buffer);
        return true;
    }

    bool AudioSource::RemoveMusic(const LongAudioBuffer& buffer)
    {
        auto it = m_LongAudioBuffers.begin();
        while (it != m_LongAudioBuffers.end())
        {
            if (it->Buffers == buffer.Buffers)
            {
                alDeleteBuffers(1, it->Buffers);
                it = m_LongAudioBuffers.erase(it);
                return true;
            }
            else
            {
                ++it; // Next Long Audio Buffer
            }
        }

        return false;
    }

    void AudioSource::Play(const AudioBuffer& buffer)
    {
        alSourcei(m_Source, AL_BUFFER, buffer.Buffer);
        alSourcePlay(m_Source);
    }

    void AudioSource::Play(const LongAudioBuffer& buffer)
    {
        alSourceRewind(m_Source);
        alSourcei(m_Source, AL_BUFFER, 0);

        alSourceQueueBuffers(m_Source, buffer.NUM_BUFFERS, &buffer.Buffers[0]);
        alSourcePlay(m_Source);

        UpdateBuffers();
    }
   
    void AudioSource::UpdateBuffers()
    {
        ALint state = AL_PLAYING;
        alGetSourcei(m_Source, AL_SOURCE_STATE, &state);

        while (state == AL_PLAYING)
        {
            ALint buffersProcessed = 0;
            alGetSourcei(m_Source, AL_BUFFERS_PROCESSED,  &buffersProcessed);
            if (buffersProcessed <= 0)
                return;

            while (buffersProcessed--)
            {
                LongAudioBuffer buffer; // Holds unqueued audio buffer
                alSourceUnqueueBuffers(m_Source, 1, buffer.Buffers);

                ALsizei dataSize = buffer.BUFFER_SIZE;

                char* data = new char[dataSize];
                std::memset(data, 0, dataSize);

                std::size_t dataSizeToCopy = buffer.BUFFER_SIZE;
                if (buffer.Cursor + buffer.BUFFER_SIZE > buffer.Data.size())
                    dataSizeToCopy = buffer.Data.size() - buffer.Cursor;

                std::memcpy(&data[0], &buffer.Data[buffer.Cursor], dataSizeToCopy);
                buffer.Cursor += dataSizeToCopy;

                if (dataSizeToCopy < buffer.BUFFER_SIZE)
                {
                    buffer.Cursor = 0;
                    std::memcpy(&data[dataSizeToCopy], &buffer.Data[buffer.Cursor], buffer.BUFFER_SIZE - dataSizeToCopy);
                    buffer.Cursor = buffer.BUFFER_SIZE - dataSizeToCopy;
                }

                alBufferData(*buffer.Buffers, buffer.Format, data, buffer.BUFFER_SIZE, buffer.SampleRate);
                alSourceQueueBuffers(m_Source, 1, buffer.Buffers);

                delete[] data;
            }

            alGetSourcei(m_Source, AL_SOURCE_STATE, &state);
            GE_CORE_TRACE("AudioSource::UpdateBuffers State: {0}", (char*)alGetString(state));
        }

    }
}