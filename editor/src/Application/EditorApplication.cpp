#include "EditorApplication.h"

#include "Layer/EditorLayer.h"
#include "../AssetManager/EditorAssetManager.h"

#include <GE/Core/EntryPoint.h>

#include <GE/Project/Project.h>
namespace GE
{
	EditorApplication::EditorApplication(const Config& config) : Application(config)
	{
		Project::NewAssetManager<EditorAssetManager>()->DeserializeAssets();
		PushLayer(new EditorLayer());
	}

	// Declared in GE::EntryPoint.h
	Application* CreateApplication(Application::CommandLineArgs args)
	{
		Application::Config appConfig;
		appConfig.Name = "Game Editor";
		appConfig.Args = args;

		//Project::New()->Save("projects/demo/demo.gproj");

		return new EditorApplication(appConfig);
	}
}
