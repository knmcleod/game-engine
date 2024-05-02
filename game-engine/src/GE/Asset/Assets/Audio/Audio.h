#pragma once

#include "GE/Asset/Asset.h"

#include <al.h>
#include <alc.h>
#include <glm/glm.hpp>

namespace GE
{
	struct AudioBuffer
	{
		enum class Type
		{
			None = 0,
			Short,
			Long
		};

		AudioBuffer() = default;

		// Bits per Sample
		uint8_t BPS = 0;
		uint8_t Channels = 2;

		int32_t SampleRate = 16;
		int32_t Format = 0;

		uint32_t Size = 0;

		Type BufferType = Type::None;

	};

	struct ShortAudioBuffer : public AudioBuffer
	{
		ShortAudioBuffer() = default;

		uint32_t Buffer = 0;
		char* Data = 0;
	};

	struct LongAudioBuffer : public AudioBuffer
	{
		LongAudioBuffer() = default;

		static const uint32_t NUM_BUFFERS = 4;
		std::size_t Cursor = Size * NUM_BUFFERS;

		uint32_t Buffers[NUM_BUFFERS];
		std::vector<char*> Data = std::vector<char*>(NUM_BUFFERS);
	};

	class AudioSource : public Asset
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

		static inline const Asset::Type GetAssetType() { return Asset::Type::AudioSource; }
		inline const Asset::Type GetType() const override { return GetAssetType(); }

		AudioSource();
		AudioSource(const Config& config, const glm::vec3& position = glm::vec3(0), const glm::vec3& velocity = glm::vec3(0));
		~AudioSource() override;

		void SetSourceValues();

		const uint32_t& GetSource() const { return m_Config.SourceID; }

		template<typename T>
		Ref<T> GetBuffer() { return static_ref_cast<T, AudioBuffer>(m_AudioBuffer); }

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