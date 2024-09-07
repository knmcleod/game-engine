#include "GE/GEpch.h"

#include "Audio.h"

#include "GE/Audio/AudioManager.h"

#include "Platform/OpenAL/Assets/OpenALAudio.h"

#include <al.h>

namespace GE
{
	Audio::Config::Config(const Config& config) : Config(config.Name, config.Channels, config.SampleRate, config.BPS, config.DataBuffer)
	{
		Format = config.Format;
		BufferIDs = config.BufferIDs;
	}

	Audio::Config::Config(const std::string& name, uint32_t channels, uint32_t sampleRate, uint32_t bps, const Buffer& buffer /*= Buffer()*/)
	{
		Name = name;
		BufferIDs = std::vector<uint32_t>();

		Channels = channels;
		SampleRate = sampleRate;
		BPS = bps;

		CalculateFormat();

		if (buffer)
			SetData(buffer);
	}

	void Audio::Config::CalculateFormat()
	{
		if (Channels == 1 && BPS == 8)
			Format = AL_FORMAT_MONO8;
		else if (Channels == 1 && BPS == 16)
			Format = AL_FORMAT_MONO16;
		else if (Channels == 2 && BPS == 8)
			Format = AL_FORMAT_STEREO8;
		else if (Channels == 2 && BPS == 16)
			Format = AL_FORMAT_STEREO16;
		else
		{
			GE_CORE_ERROR("Unrecognized wave format.\nChannels {0}\nBPS {1}\n", Channels, BPS);
		}
	}

	Ref<Audio> Audio::Create(UUID handle /* = UUID()*/, const Config& config /* = Config()*/, const uint32_t& bufferCount /* = 1*/)
	{
		switch (AudioManager::GetAPI())
		{
		case AudioManager::API::None:
			GE_CORE_ERROR("Audio API is None.");
			break;
		case AudioManager::API::OpenAL:
			return CreateRef<OpenALAudio>(handle, config, bufferCount);
			break;
		}
		return nullptr;
	}
}