#pragma once

//	--	Entry Point	--
#include "GE/EntryPoint.h"

//	-- Logging	--
#include "GE/Log/Log.h"

//	--	Application	--
#include "GE/Application/Application.h"
#include "GE/Application/Window/Layers/Layer.h"
#include "GE/Application/Window/Layers/LayerStack.h"
#include "GE/Application/Window/Layers/imgui/ImGuiLayer.h"

//	--	Timestep	--
#include "GE/Core/Timestep/Timestep.h"

//	--	Input	--
#include "GE/Input/Input.h"
#include "GE/Input/KeyCodes.h"
#include "GE/Input/MouseCodes.h"

//	--	Events	--
#include "GE/Events/MouseEvent.h"
#include "GE/Events/KeyEvent.h"
#include "GE/Events/ApplicationEvent.h"

//	--	Rendering	--
#include "GE/Rendering/RenderCommand.h"
#include "GE/Rendering/Renderer/Renderer.h"

#include "GE/Rendering/Shader/Shader.h"
#include "GE/Rendering/VertexArray/VertexArray.h"
#include "GE/Rendering/VertexArray/Buffer.h"

#include "GE/Rendering/Texture/Texture.h"

#include "GE/Rendering/Camera/OrthographicCamera.h"
