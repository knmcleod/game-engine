#include <GE/GE.h>

#include "2D/Demo2D.h"

class DemoApplication : public GE::Application
{
public:
	DemoApplication(const GE::ApplicationSpecification& specification) : Application(specification)
	{
		PushLayer(new Demo2D());
	}

private:

};

GE::Application* GE::CreateApplication(ApplicationCommandLineArgs args)
{
	ApplicationSpecification appSpec;
	appSpec.Name = "Demo";
	appSpec.CommandLineArgs = args;

	return new DemoApplication(appSpec);
}
