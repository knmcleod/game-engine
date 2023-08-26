#pragma once

namespace GE
{
	class Renderer
	{
	public:
		enum class RendererAPI
		{
			None = 0, OpenGL = 1
		};

		inline static RendererAPI GetAPI() { return s_RendererAPI; }
		inline static void SetAPI(RendererAPI api) { s_RendererAPI = api; }
	private:
		inline static RendererAPI s_RendererAPI;
	};
}

