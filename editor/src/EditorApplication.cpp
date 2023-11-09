#include <GE/GE.h>
#include "Layer/EditorLayer.h"

namespace GE
{
	class EditorApplication : public Application
	{
	public:
		EditorApplication() : Application("Editor")
		{
			PushLayer(new EditorLayer());
		}

		~EditorApplication()
		{

		}

	};

	Application* CreateApplication()
	{
		return new EditorApplication();
	}
}
