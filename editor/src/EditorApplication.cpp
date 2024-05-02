#include "Layer/EditorLayer.h"

#include <GE/Core/Application/Application.h>

namespace GE
{
	class EditorApplication : public Application
	{
	public:
		EditorApplication(const ApplicationSpecification& specification) : Application(specification)
		{
			PushLayer(new EditorLayer());
		}

	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification appSpec;
		appSpec.Name = "Game Editor";
		appSpec.CommandLineArgs = args;

		//Project::New()->Save("projects/demo/demo.gproj");

		return new EditorApplication(appSpec);
	}
}
