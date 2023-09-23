#pragma once

#include "GE/Rendering/RenderCommand.h"

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

		// Starts renderer using given camera 
		static void Start(OrthographicCamera& orthoCamera);

		// Runs shader amd vertex array at given transform
		static void Run(const Ref<Shader>& shader,
						const Ref<VertexArray>& vertexArray,
						const glm::mat4& transform = glm::mat4(1.0f));
		// Ends renderer
		static void End();

		static void OnWindowResize(uint32_t width, uint32_t height);

	private:
		struct Data
		{
			glm::mat4 ViewProjectionMatrix = glm::mat4(1.0f);
		};

		static Data* m_Data;
	};
}

