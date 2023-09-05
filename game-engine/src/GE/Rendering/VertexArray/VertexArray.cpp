#include "GE/GEpch.h"

#include "VertexArray.h"
#include "GE/_Platform/OpenGL/VertexArray/OpenGLVertexArray.h"
#include "GE/Rendering/RenderAPI.h"

namespace GE
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (RenderAPI::GetAPI())
		{
		case RenderAPI::API::None:
			GE_CORE_ASSERT(false, "RendererAPI::None is not currently supported!");
			return nullptr;
			break;
		case RenderAPI::API::OpenGL:
			return (Ref<VertexArray>) new OpenGLVertexArray();
			break;
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}