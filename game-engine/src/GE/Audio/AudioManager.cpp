#include "GE/GEpch.h"

#include "AudioManager.h"

#include "GE/Project/Project.h"

#include <al.h>

namespace GE
{
    AudioManager::API AudioManager::s_API = AudioManager::API::OpenAL;
    AudioManager::IDBuffer AudioManager::s_Sources = AudioManager::IDBuffer();
    Ref<AudioDevice> AudioManager::s_Device = nullptr;

    void AudioManager::Init()
    {
        AudioManager::s_Device = AudioDevice::Create();
    }

    void AudioManager::Shutdown()
    {
        for (const auto& id : s_Sources.m_IDBuffer)
            Stop(id);

        alDeleteSources(s_Sources.GetSize(), s_Sources);
       
    }

    void AudioManager::Stop(const uint32_t& sourceID)
    {
        if (!s_Sources.IDExists(sourceID) || !IsSourcePlaying(sourceID))
            return;
        alSourceStop(sourceID);
       
        UnbindBuffer(sourceID);
    }

    void AudioManager::Pause(const uint32_t& sourceID)
    {
        if (!IsSourcePlaying(sourceID))
            return;
        alSourcePause(sourceID);
       
    }

    void AudioManager::Update(const AudioSourceComponent& asc, const glm::vec3& position, const glm::vec3& velocity)
    {
        SetSourceValues(asc.ID, asc.Loop, asc.Pitch, asc.Gain, position, velocity);
    }

    void AudioManager::Play(const uint32_t& sourceID)
    {
        if (!s_Sources.IDExists(sourceID))
            return;

        if (!IsSourcePlaying(sourceID))
        {
            alSourcePlay(sourceID);
        }
    }

    void AudioManager::Play(const uint32_t& sourceID, const uint32_t& bufferID)
    {
        if (!s_Sources.IDExists(sourceID) || IsSourcePlaying(sourceID))
            return;

        BindBuffer(sourceID, bufferID);

        alSourcePlay(sourceID);
    }

    void AudioManager::Play(const uint32_t& sourceID, const bool& loop, const float& pitch, const float& gain, const glm::vec3& position, const glm::vec3& velocity, Ref<Audio> audioAsset)
    {
        if (!s_Sources.IDExists(sourceID) || IsSourcePlaying(sourceID))
            return;

        const Audio::Config& config = audioAsset->GetConfig();
        if (config.BufferIDs.size() <= 0)
        {
            GE_CORE_ERROR("Cannot play Audio.\n\tInvalid number of BufferIDs. {0}", config.BufferIDs.size());
            return;
        }

        SetSourceValues(sourceID, loop, pitch, gain, position, velocity);

        if (config.BufferIDs.size() == 1)
            Play(sourceID, config.BufferIDs[0]);
        else
        {
            QueueBuffers(sourceID, config.BufferIDs);
            Play(sourceID);
        }

    }

    bool AudioManager::IsSourcePlaying(const uint32_t& id)
    {
        ALint state = AL_STOPPED;
        alGetSourcei(id, AL_SOURCE_STATE, &state);
        return state == AL_PLAYING ? true : false;
    }

    bool AudioManager::GenerateSource(uint32_t& id, const bool loop, const float pitch, const float gain, const glm::vec3& position, const glm::vec3& velocity)
    {
        alGenSources(1, &id);
       
        return AddSource(id, loop, pitch, gain, position, velocity);
    }

    bool AudioManager::GenerateSource(AudioSourceComponent& asc, const glm::vec3& position, const glm::vec3& velocity)
    {
        return GenerateSource(asc.ID, asc.Loop, asc.Pitch, asc.Gain, position, velocity);
    }

    void AudioManager::RemoveSource(const uint32_t& id)
    {
        UnbindBuffer(id);
        alDeleteSources(1, &id);
       
        s_Sources.RemoveID(id);
    }

    bool AudioManager::AddSource(const AudioSourceComponent& asc, const glm::vec3& position, const glm::vec3& velocity)
    {
        return AddSource(asc.ID, asc.Loop, asc.Pitch, asc.Gain, position, velocity);
    }

    bool AudioManager::AddSource(const uint32_t& id, const bool& loop, const float& pitch, const float& gain, const glm::vec3& position, const glm::vec3& velocity)
    {
        if (id <= 0)
            return false;

        SetSourceValues(id, loop, pitch, gain, position, velocity);
        return s_Sources.AddID(id);
    }

    void AudioManager::QueueBuffers(const uint32_t& sourceID, const std::vector<uint32_t> ids)
    {
        alSourceQueueBuffers(sourceID, ids.size(), ids.data());
       
    }

    void AudioManager::BindBuffer(const uint32_t& sourceID, const uint32_t& bufferID)
    {
        if (!s_Sources.IDExists(sourceID))
            return;
        UnbindBuffer(sourceID);
        alSourcei(sourceID, AL_BUFFER, bufferID);
       
    }

    void AudioManager::UnbindBuffer(const uint32_t& sourceID)
    {
        if (!s_Sources.IDExists(sourceID))
            return;
        alSourcei(sourceID, AL_BUFFER, 0);
       
    }

    void AudioManager::SetSourceValues(const uint32_t& id, const bool& loop, const float& pitch, const float& gain, const glm::vec3& position, const glm::vec3& velocity)
    {
        alSourcei(id, AL_LOOPING, loop);
        alSourcef(id, AL_PITCH, pitch);
        alSourcef(id, AL_GAIN, gain);
        alSource3f(id, AL_POSITION, position.x, position.y, position.z);
        alSource3f(id, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    }

    void AudioManager::SetListenerValues(const glm::vec3& position, const glm::vec3& velocity)
    {
        alListener3f(AL_POSITION, position.x, position.z, position.y);
        alListener3f(AL_VELOCITY, velocity.x, velocity.z, velocity.y);
    }
}