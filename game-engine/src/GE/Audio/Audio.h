#pragma once

#include <al.h>
#include <alc.h>

#include <glm/glm.hpp>

namespace GE
{
	struct AudioBuffer
	{
		friend class AudioManager;
		friend class AudioSource;

		ALuint m_AudioBuffer = 0;

		uint8_t Channels;
		int32_t SampleRate;
		// Bits per Sample
		uint8_t BPS;
		ALsizei Size;

		char* Data;
		ALenum Format;
	};

	class AudioDevice
	{
	public:
		AudioDevice();
		~AudioDevice();
	private:
		ALCdevice* m_Device = nullptr;
		ALCcontext* m_Context = nullptr;
	};

	class AudioSource : public Asset
	{
	public:
		AudioSource();
		~AudioSource();

		void Play(AudioBuffer audioBuffer);

		virtual AssetType GetType() override { return AssetType::AudioSource; }
	private:
		bool m_Loop = false;
		float m_Pitch = 1.0f;
		float m_Gain = 1.0f;
		glm::vec3 m_Position = glm::vec3();
		glm::vec3 m_Vecloity = glm::vec3();

		ALuint m_AudioSource = 0;
	};

	class AudioManager
	{
	public:
		AudioManager();
		~AudioManager();

		bool AddSound(const std::filesystem::path& filePath);
		bool RemoveSound(AudioBuffer audioBuffer);

		// Plays all current Audio Sounds
		void PlaySounds();

	private:
		bool LoadWavFile(std::ifstream& file, AudioBuffer& audioBuffer);

		Ref<AudioDevice> m_AudioDevice = nullptr;
		Ref<AudioSource> m_AudioSource = nullptr;
		std::vector<AudioBuffer> m_AudioBuffers;
	};
}
