#pragma once

#include "AudioUtils.h"

#include "GE/Asset/Asset.h"

#include <glm/glm.hpp>

namespace GE
{
	class AudioSource : public Asset
	{
	public:
		AudioSource();
		~AudioSource();

		const uint32_t& GetSource() { return m_Source; }

		bool AddSound(AudioBuffer& buffer);
		bool RemoveSound(const AudioBuffer& buffer);

		bool AddMusic(LongAudioBuffer& buffer);
		bool RemoveMusic(const LongAudioBuffer& buffer);

		void Play(const AudioBuffer& audioBuffer);
		void Play(const LongAudioBuffer& audioBuffer);

		void UpdateBuffers();

		virtual AssetType GetType() override { return AssetType::AudioSource; }
	private:
		ALuint m_Source = 0;

		bool m_Loop = false;
		float m_Pitch = 1.0f;
		float m_Gain = 1.0f;
		glm::vec3 m_Position = glm::vec3();
		glm::vec3 m_Velocity = glm::vec3();

		std::vector<AudioBuffer> m_AudioBuffers;
		std::vector<LongAudioBuffer> m_LongAudioBuffers;
	};

	class AudioListener : public Asset
	{
	public:
		virtual AssetType GetType() override { return AssetType::AudioListener; }
	private:
	};

}
