#pragma once

#pragma region Asset
#include "GE/Asset/Assets/Asset.h"
#include "GE/Asset/Assets/Audio/Audio.h"
#include "GE/Asset/Assets/Font/Font.h"
#include "GE/Asset/Assets/Scene/Scene.h"
#include "GE/Asset/Assets/Scene/Components/Components.h"
#include "GE/Asset/Assets/Textures/Texture.h"

#include "GE/Asset/Pack/AssetPack.h"
#include "GE/Asset/Registry/AssetRegistry.h"

#include "GE/Asset/AssetManager.h"
#include "GE/Asset/RuntimeAssetManager.h"
#pragma endregion

#pragma region Audio
#include "GE/Audio/AudioManager.h"
#pragma endregion


#pragma region Core
//	-- Core	--
#include "GE/Core/Core.h"

//	--	Application	--
#include "GE/Core/Application/Application.h"
#include "GE/Core/Application/Layers/Layer.h"
#include "GE/Core/Application/Layers/LayerStack.h"

//	--	Events	--
#include "GE/Core/Events/Event.h"
#include "GE/Core/Events/MouseEvent.h"
#include "GE/Core/Events/KeyEvent.h"
#include "GE/Core/Events/ApplicationEvent.h"

//	--	Input	--
#include "GE/Core/Input/Input.h"

//	--	Time	--
#include "GE/Core/Time/Timestep.h"

//	--	UUID	--
#include "GE/Core/UUID/UUID.h"
#pragma endregion

#pragma region Project
#include "GE/Project/Project.h"
#pragma endregion

#pragma region Rendering
//	--	Renderer	--
#include "GE/Rendering/RenderCommand.h"
#include "GE/Rendering/Renderer/2D/Renderer2D.h"

#include "GE/Rendering/Framebuffers/Framebuffer.h"

//	--	Shader	--
#include "GE/Rendering/Shader/Shader.h"
#include "GE/Rendering/VertexArray/VertexArray.h"
#include "GE/Rendering/VertexArray/Buffers/VertexBuffer.h"
#include "GE/Rendering/VertexArray/Buffers/IndexBuffer.h"

//	--	Camera	--
#include "GE/Rendering/Camera/Camera.h"
#include "GE/Rendering/Camera/Controller/CameraController.h"
#pragma endregion

#pragma region Scripting
#include "GE/Scripting/Scripting.h"
#pragma endregion
