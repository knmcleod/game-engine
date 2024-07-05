#include "GE/GEpch.h"

#include "IndexBuffer.h"

#include "GE/Rendering/RenderAPI.h"

#include "Platform/OpenGL/VertexArray/Buffers/OpenGLIndexBuffer.h"

namespace GE
{
	Ref<IndexBuffer> IndexBuffer::Create(uint32_t size)
	{
		switch (RenderAPI::GetAPI())
		{
		case RenderAPI::API::None:
			GE_CORE_ASSERT(false, "RendererAPI::None is not currently supported!");
			return nullptr;
			break;
		case RenderAPI::API::OpenGL:
			return CreateRef<OpenGLIndexBuffer>(size);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
