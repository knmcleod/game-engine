#include "GE/Audio/AudioDevice.h"

#include <alc.h>

namespace GE
{
	class OpenALAudioDevice : public AudioDevice
	{
		friend class Audio;
	public:
		OpenALAudioDevice();
		~OpenALAudioDevice() override;

		inline const std::string& GetName() override { return m_Name; }
	private:
		void Init() override;
	private:
		ALCdevice* m_Device = nullptr;
		ALCcontext* m_Context = nullptr;

		std::string m_Name = std::string("NewDevice");
	};
}