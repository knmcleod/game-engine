#pragma once

#pragma region Core
//	-- Core	--
#include "GE/Core/Core.h"

//	--	Application	--
#include "GE/Core/Application/Application.h"
#include "GE/Core/Application/Layers/Layer.h"
#include "GE/Core/Application/Layers/LayerStack.h"
#include "GE/Core/ImGui/ImGuiLayer.h"

//	-- Debug	--
#include "GE/Core/Debug/Instrumentor.h"
#include "GE/Core/Debug/Log/Log.h"

//	--	Events	--
#include "GE/Core/Events/MouseEvent.h"
#include "GE/Core/Events/KeyEvent.h"
#include "GE/Core/Events/ApplicationEvent.h"

//	--	Input	--
#include "GE/Core/Input/Input.h"
#include "GE/Core/Input/KeyCodes.h"
#include "GE/Core/Input/MouseCodes.h"

//	--	Time	--
#include "GE/Core/Time/Time.h"
#pragma endregion

#pragma region Rendering
//	--	Rendering	--
#include "GE/Rendering/RenderCommand.h"
#include "GE/Rendering/Renderer/Renderer.h"
#include "GE/Rendering/Renderer/2D/Renderer2D.h"

#include "GE/Rendering/Shader/Shader.h"
#include "GE/Rendering/VertexArray/VertexArray.h"
#include "GE/Rendering/VertexArray/Buffer.h"

#include "GE/Rendering/Textures/Texture.h"

#include "GE/Rendering/Framebuffers/Framebuffer.h"

#include "GE/Rendering/Camera/OrthographicCamera.h"
#include "GE/Rendering/Camera/Controller/OrthographicCameraController.h"
#pragma endregion
