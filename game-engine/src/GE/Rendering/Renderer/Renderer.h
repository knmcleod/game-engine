#pragma once

#include "GE/Rendering/RenderAPI.h"

#include "GE/Rendering/Camera/OrthographicCamera.h"

namespace GE
{
	class Renderer
	{
	public:
		// Returns RenderAPI in use
		inline static RenderAPI::API GetAPI() { return RenderAPI::GetAPI(); }

		static void Init();
		static void ShutDown();
		
	};
}

