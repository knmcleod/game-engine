#pragma once

#include "AudioDevice.h"

#include "GE/Asset/Assets/Audio/Audio.h"
#include "GE/Asset/Assets/Scene/Components/Components.h"

#include <glm/glm.hpp>

namespace GE
{
	class AudioManager
	{
		friend class AssetSerializer;
	public:
		enum class API
		{
			None = 0,
			OpenAL
		};

		struct IDBuffer
		{
			friend class AudioManager;
			operator uint32_t* () const { return (uint32_t*)m_IDBuffer.data(); }

			IDBuffer()
			{
				m_IDBuffer = std::vector<uint32_t>();
			}

			~IDBuffer()
			{
				m_IDBuffer.clear();
			}

			bool IDExists(const uint32_t& id)
			{
				for (const auto& currentID : m_IDBuffer)
				{
					if (currentID == id)
						return true;
				}
				return false;
			}

			bool AddID(const uint32_t& id)
			{
				if (IDExists(id))
					return false;
				m_IDBuffer.push_back(id);
				return true;
			}

			void RemoveID(const uint32_t& id)
			{
				if (!IDExists(id))
					return;

				auto it = std::find(m_IDBuffer.begin(), m_IDBuffer.end(), id);
				if (it != m_IDBuffer.end())
					m_IDBuffer.erase(it);
			}

			const size_t GetSize()
			{
				return m_IDBuffer.size();
			}
		private:
			// ALuint = unsigned 32-bit integer
			std::vector<uint32_t> m_IDBuffer;
		};
		
		static const std::size_t BUFFER_SIZE = 65536; // 32kb of data in each buffer

		static void Init();
		static void Shutdown();

		static void Stop(const uint32_t& sourceID);
		static void Pause(const uint32_t& sourceID);
		static void UpdateSource(const AudioSourceComponent& asc, const glm::vec3& position, const glm::vec3& velocity);
		static void UpdateListener(const AudioListenerComponent& asc, const glm::vec3& position, const glm::vec3& velocity);

		static void Play(const uint32_t& sourceID);
		static void Play(const uint32_t& sourceID, const uint32_t& bufferID);
		static void Play(const uint32_t& sourceID, const bool& loop, const float& pitch, const float& gain, const glm::vec3& position, const glm::vec3& velocity, Ref<Audio> audioAsset);

		static bool IsSourcePlaying(const uint32_t& ID);

		static bool GenerateSource(uint32_t& id, const bool loop = false, const float pitch = 1.0f, const float gain = 1.0f, const glm::vec3& position = glm::vec3(0.0), const glm::vec3& velocity = glm::vec3(0.0));
		static bool GenerateSource(AudioSourceComponent& asc, const glm::vec3& position, const glm::vec3& velocity);
		static void RemoveSource(const uint32_t& id);

		static void QueueBuffers(const uint32_t& sourceID, const std::vector<uint32_t> ids);

		static const std::string& GetDeviceName() { return s_Device->GetName(); }
		static Ref<AudioDevice> GetDevice() { return s_Device; }
		static API GetAPI() { return s_API; }
	private:
		static void SetSourceValues(const uint32_t& id, const bool& loop, const float& pitch, const float& gain, const glm::vec3& position = glm::vec3(0.0), const glm::vec3& velocity = glm::vec3(0.0));
		static void SetListenerValues(const glm::vec3& position, const glm::vec3& velocity);

		static bool AddSource(const AudioSourceComponent& asc, const glm::vec3& position = glm::vec3(0.0), const glm::vec3& velocity = glm::vec3(0.0));
		static bool AddSource(const uint32_t& id, const bool& loop = false, const float& pitch = 1.0f, const float& gain = 1.0f, const glm::vec3& position = glm::vec3(0.0), const glm::vec3& velocity = glm::vec3(0.0));

		static void BindBuffer(const uint32_t& sourceID, const uint32_t& bufferID);
		static void UnbindBuffer(const uint32_t& sourceID);
	private:
		static API s_API;
		static IDBuffer s_Sources;
		static Ref<AudioDevice> s_Device;
	};
}