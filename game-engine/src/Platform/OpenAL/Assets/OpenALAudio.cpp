#include "GE/GEpch.h"

#include "OpenALAudio.h"

#include "GE/Audio/AudioManager.h"

#include <al.h>

namespace GE
{

    OpenALAudio::OpenALAudio(UUID handle, const Config& config, const uint32_t& bufferCount) : Audio(handle, Asset::Type::Audio)
    {
        m_Config = Config(config);
        GenerateBuffers(bufferCount);
        SetBufferData();
    }

    OpenALAudio::~OpenALAudio()
    {
        ClearBuffers();
        m_Config.ReleaseData();
    }

    const float OpenALAudio::GetDurationInSeconds()
    {
        float totalDuration = 0.0f;

        for (const auto& id : m_Config.BufferIDs)
        {
            ALint sizeInBytes;
            ALint channels;
            ALint bits;

            alGetBufferi(id, AL_SIZE, &sizeInBytes);
            alGetBufferi(id, AL_CHANNELS, &channels);
            alGetBufferi(id, AL_BITS, &bits);

            float lengthInSamples = sizeInBytes * 8.0f / (channels * bits);

            ALint frequency;

            alGetBufferi(id, AL_FREQUENCY, &frequency);

            totalDuration += (float)lengthInSamples / (float)frequency;
        }
        
        return totalDuration;
    }

    const uint64_t OpenALAudio::GetBufferCount() const
    {
        return m_Config.DataBuffer.GetSize() > AudioManager::BUFFER_SIZE ? (m_Config.DataBuffer.GetSize() / AudioManager::BUFFER_SIZE) + 1 : 1;
    }

    const std::vector<uint32_t>& OpenALAudio::GenerateBuffers(const uint32_t& count)
    {
        ClearBuffers();
        for (uint32_t i = 0; i < count; i++)
        {
            ALuint newBufferID = 0;
            alGenBuffers(1, &newBufferID);
            AddID(newBufferID);
        }
        return m_Config.BufferIDs;
    }

    void OpenALAudio::SetBufferData()
    {
        if (m_Config.DataBuffer)
        {
            for (std::size_t i = 0; i < m_Config.BufferIDs.size(); ++i)
            {
                if (m_Config.DataBuffer.GetSize() < i * AudioManager::BUFFER_SIZE)
                    break;
                alBufferData(m_Config.BufferIDs.at(i), m_Config.Format, &m_Config.DataBuffer.As<uint8_t>()[i * AudioManager::BUFFER_SIZE], AudioManager::BUFFER_SIZE, m_Config.SampleRate);
            }
        }
    }

    void OpenALAudio::ClearBuffers()
    {
        alDeleteBuffers((ALsizei)m_Config.BufferIDs.size(), m_Config.BufferIDs.data());
        m_Config.BufferIDs.clear();
    }

    void OpenALAudio::AddID(const uint32_t& id)
    {
        m_Config.BufferIDs.push_back(id);
    }
}