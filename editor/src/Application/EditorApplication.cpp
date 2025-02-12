#include "EditorApplication.h"

#include "Layer/EditorLayer.h"
#include "Layer/EditorLayerStack.h"
#include "../AssetManager/EditorAssetManager.h"

#include <GE/Core/EntryPoint.h>

#include <GE/Project/Project.h>
#include <GE/Scripting/Scripting.h>

namespace GE
{
	EditorApplication::EditorApplication(const Config& config) : Application(config)
	{
		p_LayerStack = CreateRef<EditorLayerStack>();
		Project::NewAssetManager<EditorAssetManager>()->DeserializeAssets();
		// Attach Layers added through deserialization
		AttachAllLayers();
	}

	// Declared in GE::EntryPoint.h
	Application* CreateApplication(Application::CommandLineArgs args)
	{
		Application::Config appConfig;
		appConfig.Name = "Game Editor";
		appConfig.Args = args;
		// TODO : Use editor.gproj
		// args[1] = project filePath = "projects/demo/demo.gproj"
		// To Create new Project: 
		// Project::New()->Save(args[1]);

		return new EditorApplication(appConfig);
	}
}
