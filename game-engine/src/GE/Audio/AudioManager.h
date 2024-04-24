#pragma once

#include "Audio.h"
#include "AudioDevice.h"

//#include "GE/Scene/Components/Components.h"

namespace GE
{

	class AudioManager
	{
	public:
		AudioManager();
		~AudioManager();

		// Adds wav file to specific audio source via AudioBuffer
		bool AddSound(Ref<AudioSource> source, const std::filesystem::path& filePath);
		// Adds wav file to new AudioSource via AudioBuffer
		bool AddSound(const std::filesystem::path& filePath);

		// Adds wav file to specific audio source via LongAudioBuffer
		bool AddMusic(Ref<AudioSource> source, const std::filesystem::path& filePath);
		// Adds wav file to new AudioSource via LongAudioBuffer
		bool AddMusic(const std::filesystem::path& filePath);

		bool RemoveSource(Ref<AudioSource> source);

		void PlaySounds();
		void PlayMusic();
	private:
		bool LoadWavFile(std::ifstream& file,
			std::uint8_t& channels,
			std::int32_t& sampleRate,
			std::uint8_t& bitsPerSample,
			ALsizei& size);

		char* LoadWav(const std::filesystem::path& filePath,
			std::uint8_t& channels,
			std::int32_t& sampleRate,
			std::uint8_t& bitsPerSample,
			ALsizei& size);

		bool LoadWav(const std::filesystem::path& filePath,
			LongAudioBuffer& buffer);

		Ref<AudioDevice> m_AudioDevice = nullptr;
		std::vector<Ref<AudioSource>> m_AudioSources;

		std::vector<AudioBuffer> m_AudioBuffers;
		std::vector<LongAudioBuffer> m_LongAudioBuffers;

	};
}

