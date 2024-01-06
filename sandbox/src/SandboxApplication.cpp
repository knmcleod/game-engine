#include <GE/GE.h>

#include "2D/Sandbox2D.h"

class SandboxApplication : public GE::Application
{
public:
	SandboxApplication(const GE::ApplicationSpecification& specification) : Application(specification)
	{
		PushLayer(new Sandbox2D());
	}

private:

};

GE::Application* GE::CreateApplication()
{
	ApplicationSpecification spec;
	spec.Name = "Game Editor Application";
	return new SandboxApplication(spec);
}
