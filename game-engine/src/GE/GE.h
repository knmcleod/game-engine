#pragma once

#pragma region Core
//	-- Core	--
#include "GE/Core/Core.h"

//	--	Application	--
#include "GE/Core/Application/Application.h"
#include "GE/Core/Application/Layers/Layer.h"
#include "GE/Core/Application/Layers/LayerStack.h"
#include "GE/ImGui/ImGuiLayer.h"

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

//	--	Util	--
#include "GE/Core/Util/PlatformUtils.h"

//	--	UUID	--
#include "GE/Core/UUID/UUID.h"
#pragma endregion

#pragma region Project
#include "GE/Project/Project.h"
#pragma endregion

#pragma region Rendering
//	--	Renderer	--
#include "GE/Rendering/RenderCommand.h"
#include "GE/Rendering/Renderer/Renderer.h"
#include "GE/Rendering/Renderer/2D/Renderer2D.h"

#include "GE/Rendering/Framebuffers/Framebuffer.h"

//	--	Shader	--
#include "GE/Rendering/Shader/Shader.h"
#include "GE/Rendering/VertexArray/VertexArray.h"
#include "GE/Rendering/VertexArray/Buffer.h"

//	--	Texture	--
#include "GE/Rendering/Textures/Texture.h"
#include "GE/Rendering/Textures/SubTexture.h"

//	--	Camera	--
#include "GE/Rendering/Camera/Camera.h"
#include "GE/Rendering/Camera/Controller/CameraController.h"
#pragma endregion

#pragma region Scene
#include "GE/Scene/Scene.h"
#include "GE/Scene/Entity/Entity.h"
#include "GE/Scene/Entity/ScriptableEntity.h"
#include "GE/Scene/Components/Components.h"

//	--	Camera	--
#include "GE/Scene/Camera/SceneCamera.h"

//	--	Serializer	--
#include "GE/Scene/Serializer/SceneSerializer.h"
#pragma endregion
