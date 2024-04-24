#pragma once

#include <alc.h>

namespace GE
{
	class AudioDevice
	{
	public:
		AudioDevice();
		~AudioDevice();
	private:
		ALCdevice* m_Device = nullptr;
		ALCcontext* m_Context = nullptr;
	};

}

