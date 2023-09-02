#pragma once

#include "GE/Rendering/RenderCommand.h"

#include "GE/Rendering/Camera/OrthographicCamera.h"

namespace GE
{
	class Renderer
	{
	public:
		inline static RenderAPI::API GetAPI() { return RenderAPI::GetAPI(); }

		static void Start(OrthographicCamera& orthoCamera);
		static void Run(const std::shared_ptr<Shader>& shader,
						const std::shared_ptr<VertexArray>& vertexArray,
						const glm::mat4& transform = glm::mat4(1.0f));
		static void End();

	private:
		struct Data
		{
			glm::mat4 ViewProjectionMatrix = glm::mat4(1.0f);
		};

		static Data* m_Data;
	};
}

