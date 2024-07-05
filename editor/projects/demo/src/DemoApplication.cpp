#include <GE/GE.h>

#include "2D/Demo2D.h"

class DemoApplication : public GE::Application
{
public:
	DemoApplication(const GE::Application::Specification& specification) : Application(specification)
	{
		PushLayer(new Demo2D());
	}

private:

};

GE::Application* GE::CreateApplication(Application::CommandLineArgs args)
{
	Application::Specification appSpec;
	appSpec.Name = "Demo";
	appSpec.Args = args;

	return new DemoApplication(appSpec);
}
