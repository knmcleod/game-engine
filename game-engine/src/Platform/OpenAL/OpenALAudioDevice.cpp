#include "GE/GEpch.h"

#include "OpenALAudioDevice.h"

namespace GE
{
    OpenALAudioDevice::OpenALAudioDevice()
    {
        Init();
    }

    OpenALAudioDevice::~OpenALAudioDevice()
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(m_Context);

		alcCloseDevice(m_Device);
	}

	void OpenALAudioDevice::Init()
	{
        m_Device = alcOpenDevice(nullptr); // Returns default device
        if (!m_Device)
        {
            GE_CORE_ERROR("Could not open audio device!");
            return;
        }

        m_Context = alcCreateContext(m_Device, nullptr); // Returns default context
        if (!m_Context)
        {
            GE_CORE_ERROR("Could not create audio device context!");
            return;
        }

        if (!alcMakeContextCurrent(m_Context))
        {
            GE_CORE_ERROR("Could not set audio device context!");
            return;
        }

        const ALCchar* name = nullptr;
        if (alcIsExtensionPresent(m_Device, "ALC_ENUMERATE_ALL_EXT"))
            name = alcGetString(m_Device, ALC_ALL_DEVICES_SPECIFIER);

        if (!name || alcGetError(m_Device) != ALC_NO_ERROR)
            name = alcGetString(m_Device, ALC_DEVICE_SPECIFIER);

        m_Name = std::string(name);
        GE_CORE_INFO("Opened Audio Device: {0}", m_Name.c_str());
	}
}
