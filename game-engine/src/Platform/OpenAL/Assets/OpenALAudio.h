#include "GE/Asset/Assets/Audio/Audio.h"

namespace GE
{
	class OpenALAudio : public Audio
	{
	public:
		OpenALAudio(UUID handle, const Config& config, const uint32_t& bufferCount = 1);
		~OpenALAudio() override;

		Ref<Asset> GetCopy() override { return nullptr; }

		/*
		* returns front of BufferIDs
		*/
		const uint32_t& GetID() const override { return m_Config.BufferIDs.front(); }
		const std::vector<uint32_t>& GetAllID() const { return m_Config.BufferIDs; }
		const Config& GetConfig() const override { return m_Config; }
		const uint32_t GetBufferCount() const override;

		const float GetDurationInSeconds() override;
		const std::vector<uint32_t>& GenerateBuffers(const uint32_t& count = 1) override; 
		void SetBufferData() override;
		void ClearBuffers() override;
	private:
		void AddID(const uint32_t& id) override;
	private:
		Config m_Config;
	};
}