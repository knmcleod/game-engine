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

		//	Clears buffers
		virtual void Clear() = 0;

		//	Draws elements
		virtual void DrawIndices(const Ref<VertexArray>& vertexArray) = 0;
	private:
		static API s_API;
	};

}