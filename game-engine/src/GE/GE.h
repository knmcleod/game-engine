#pragma once

//	-- Logging	--
#include "GE/Core/Log/Log.h"

//	--	Application	--
#include "GE/Core/Application/Application.h"
#include "GE/Core/Application/Window/Layers/Layer.h"
#include "GE/Core/Application/Window/Layers/LayerStack.h"
#include "GE/Core/Application/Window/Layers/imgui/ImGuiLayer.h"

//	--	Timestep	--
#include "GE/Core/Timestep/Timestep.h"

//	--	Input	--
#include "GE/Core/Input/Input.h"
#include "GE/Core/Input/KeyCodes.h"
#include "GE/Core/Input/MouseCodes.h"

//	--	Events	--
#include "GE/Core/Events/MouseEvent.h"
#include "GE/Core/Events/KeyEvent.h"
#include "GE/Core/Events/ApplicationEvent.h"

//	--	Rendering	--
#include "GE/Rendering/RenderCommand.h"
#include "GE/Rendering/Renderer/Renderer.h"
#include "GE/Rendering/Renderer/2D/Renderer2D.h"

#include "GE/Rendering/Shader/Shader.h"
#include "GE/Rendering/VertexArray/VertexArray.h"
#include "GE/Rendering/VertexArray/Buffer.h"

#include "GE/Rendering/Texture/Texture.h"

#include "GE/Rendering/Camera/OrthographicCamera.h"
#include "GE/Rendering/Camera/Controller/OrthographicCameraController.h"