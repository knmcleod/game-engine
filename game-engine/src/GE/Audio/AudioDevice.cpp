#include "GE/GEpch.h"

#include "AudioDevice.h"

#include "AudioManager.h"
#include "Platform/OpenAL/OpenALAudioDevice.h"

namespace GE
{
    Ref<AudioDevice> AudioDevice::Create()
    {
        switch (AudioManager::GetAPI())
        {
        case AudioManager::API::None:
            break;
        case AudioManager::API::OpenAL:
            return CreateRef<OpenALAudioDevice>();
            break;
        }
        return nullptr;
    }
}
