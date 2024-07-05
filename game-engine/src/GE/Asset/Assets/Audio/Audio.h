#pragma once

#include "GE/Asset/Assets/Asset.h"

#include <al.h>
#include <alc.h>
#include <glm/glm.hpp>

namespace GE
{
	struct AudioBuffer
	{
		// Bits per Sample
		uint8_t BPS = 0;
		uint8_t Channels = 2;

		int32_t SampleRate = 16;
		int32_t Format = 0;

		const static uint8_t NUM_BUFFERS = 4;
		uint32_t Buffers[NUM_BUFFERS];
		uint64_t Cursor = Size * NUM_BUFFERS;

		// Size of Data read from .wav file
		uint32_t Size = 0;
		// Data read from .wav file
		uint8_t* Data = nullptr;
	};

	class AudioClip : public Asset
	{
		friend class AssetSerializer;
	public:
		struct Config
		{
			uint32_t SourceID = 0;
			bool Loop = false;
			float Pitch = 1.0f;
			float Gain = 1.0f;
		};

		AudioClip();
		AudioClip(const Config& config, const glm::vec3& position = glm::vec3(0), const glm::vec3& velocity = glm::vec3(0));
		~AudioClip() override;

		Ref<Asset> GetCopy() override;
		uint64_t GetByteArray(void* buffer = nullptr, uint64_t bufferSize = 0) override;

		void SetSourceValues();

		const uint32_t& GetSource() const { return m_Config.SourceID; }

		Ref<AudioBuffer> GetBuffer() { return m_AudioBuffer; }

		void Play(Config& audioConfig = Config(), const glm::vec3& position = glm::vec3(0), const glm::vec3& velocity = glm::vec3(0));
	private:
		Config m_Config;
		glm::vec3 m_Position = glm::vec3(0);
		glm::vec3 m_Velocity = glm::vec3(0);

		Ref<AudioBuffer> m_AudioBuffer;
	};

	class AudioListener
	{
	public:
		AudioListener();
		~AudioListener();
	private:
		ALCdevice* m_Device = nullptr;
		ALCcontext* m_Context = nullptr;
	};
}