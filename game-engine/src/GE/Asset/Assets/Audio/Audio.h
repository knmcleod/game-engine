#pragma once

#include "../Asset.h"

#include "GE/Core/Memory/Buffer.h"

namespace GE
{
	class Audio : public Asset
	{
	public:
		struct Config
		{
			std::vector<uint32_t> BufferIDs;

			uint32_t Channels = 2;
			uint32_t SampleRate = 16;
			uint32_t BPS = 0; // Bits per Sample
			uint32_t Format = 0;

			Buffer DataBuffer = 0;

			Config() = default;
			Config(const Config& config) : Config(config.Channels, config.SampleRate, config.BPS, config.DataBuffer)
			{
				Format = config.Format;
				BufferIDs = config.BufferIDs;
			}
			Config(uint32_t channels, uint32_t sampleRate, uint32_t bps, const Buffer& buffer /*= Buffer()*/)
			{
				BufferIDs = std::vector<uint32_t>();

				Channels = channels;
				SampleRate = sampleRate;
				BPS = bps;

				CalculateFormat();

				if (buffer)
					SetData(buffer);
			}
			~Config() = default;

			void SetData(const Buffer buffer)
			{
				ReleaseData();
				DataBuffer = Buffer(buffer.As<void>(), buffer.GetSize());
			}

			void ReleaseData()
			{
				if(DataBuffer)
					DataBuffer.Release();
			}

			void CalculateFormat();
		};

		static Ref<Audio> Create(UUID handle = UUID(), const Config& config = Config(), const uint32_t& bufferCount = 1);

		Audio(UUID handle, Asset::Type type) : Asset(handle, type) {}
		virtual ~Audio() override {}

		virtual const uint32_t& GetID() const = 0;
		virtual const Config& GetConfig() const = 0;
		virtual const uint64_t GetBufferCount() const = 0;
		virtual const float GetDurationInSeconds() = 0;

		virtual const std::vector<uint32_t>& GenerateBuffers(const uint32_t& count = 1) = 0;
		virtual void ClearBuffers() = 0;
	protected:
		/*
		* Use after Config is set.
		*/
		virtual void SetBufferData() = 0;
		/*
		* ID should already be generated
		*/
		virtual void AddID(const uint32_t& id) = 0;
	};
}