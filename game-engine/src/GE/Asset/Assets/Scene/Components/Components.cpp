#include "GE/GEpch.h"

#include "Components.h"

#include "GE/Audio/AudioManager.h"

#include "GE/Project/Project.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace GE
{
#pragma region Transform
	const glm::mat4 TransformComponent::GetTransform(glm::vec3& translationOffset, glm::vec3& scaleOffset) const
	{
		glm::mat4 identityMat4 = glm::mat4(1.0f);
		glm::mat4 rotation = glm::rotate(identityMat4, Rotation.x, { 1, 0, 0 })
			* glm::rotate(identityMat4, Rotation.y, { 0, 1, 0 })
			* glm::rotate(identityMat4, Rotation.z, { 0, 0, 1 });

		return glm::translate(identityMat4, Translation + translationOffset)
			* rotation * glm::scale(identityMat4, Scale * scaleOffset);
	}
#pragma endregion

#pragma region AudioSource
	void AudioSourceComponent::Play(const glm::vec3& position, const glm::vec3& velocity) const
	{
		Ref<Audio> audio = Project::GetAsset<Audio>(AssetHandle);
		if (!audio || audio->GetType() != Asset::Type::Audio)
			return;

		AudioManager::Play(ID, Loop, Pitch, Gain, position, velocity, audio);
	}
	void AudioSourceComponent::Pause() const
	{
		AudioManager::Pause(ID);
	}

	void AudioSourceComponent::Stop() const
	{
		AudioManager::Stop(ID);
	}

	bool AudioSourceComponent::IsPlaying() const
	{
		return AudioManager::IsSourcePlaying(ID);
	}

#pragma endregion

	AudioListenerComponent::AudioListenerComponent()
	{
		Device = AudioManager::GetDevice();
	}

}