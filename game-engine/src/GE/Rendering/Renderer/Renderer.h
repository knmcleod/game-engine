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
		static void Run(const std::shared_ptr<Shader>& shader, std::shared_ptr<VertexArray>& vertexArray);
		static void End();

	private:
		struct Data
		{
			glm::mat4 ViewProjectionMatrix;
		};

		static Data* m_Data;
	};
}

