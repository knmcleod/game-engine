#include <GE/GE.h>

#include "2D/Sandbox2D.h"

class SandboxApplication : public GE::Application
{
public:
	SandboxApplication()
	{
		PushLayer(new Sandbox2D());
	}

	~SandboxApplication()
	{

	}

private:

};

GE::Application* GE::CreateApplication()
{
	return new SandboxApplication();
}
