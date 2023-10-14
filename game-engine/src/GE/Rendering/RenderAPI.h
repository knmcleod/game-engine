#pragma once
#include "GE/Rendering/VertexArray/VertexArray.h"

namespace GE
{
	class RenderAPI
	{
	public:

		enum class API
		{
			None = 0, OpenGL = 1
		};

		inline static API GetAPI() { return s_API; }
		inline static void SetAPI(API api) { s_API = api; }
			
		virtual void Init() = 0;
		virtual void SetViewport(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height) = 0;

		//	Clears buffers
		virtual void Clear() = 0;

		virtual void SetClearColor(const glm::vec4& color) = 0;

		//	Draws elements
		virtual void DrawIndices(const Ref<VertexArray>& vertexArray) = 0;
	private:
		static API s_API;
	};

}