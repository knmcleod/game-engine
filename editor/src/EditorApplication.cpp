#include <GE/GE.h>

#include "Layer/EditorLayer.h"

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

	Application* CreateApplication()
	{
		ApplicationSpecification spec;
		spec.Name = "Game Editor Application";
		return new EditorApplication(spec);
	}
}
