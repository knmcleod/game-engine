#pragma once

#include "GE/Core/Core.h"

namespace GE
{
	class AudioDevice
	{
	public:
		static Ref<AudioDevice> Create();

		AudioDevice() = default;
		virtual ~AudioDevice() {}

		virtual void Init() = 0;
		virtual const std::string& GetName() = 0;
	};
}