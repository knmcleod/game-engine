#pragma once
#include "GE/Rendering/VertexArray/VertexArray.h"

namespace GE
{
	class RenderAPI
	{
		friend class RenderCommand;
	public:		
		enum class API
		{
			None = 0, OpenGL = 1
		};
		inline static API GetAPI() { return s_API; }
	
	private:
		virtual void Init() = 0;
		//	Clears buffers
		virtual void Clear() = 0;

		virtual void SetViewport(uint32_t xOffset, uint32_t yOffset, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void SetLineWidth(float thickness) = 0;

		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;
		virtual void DrawIndices(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
	
		inline static void SetAPI(API api) { s_API = api; }
	private:
		static API s_API;
	};

}