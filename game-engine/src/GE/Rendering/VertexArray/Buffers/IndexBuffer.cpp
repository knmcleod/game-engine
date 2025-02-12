#include "GE/GEpch.h"

#include "IndexBuffer.h"

#include "GE/Rendering/Renderer/Renderer.h"

#include "Platform/OpenGL/VertexArray/Buffers/OpenGLIndexBuffer.h"

namespace GE
{
	Ref<IndexBuffer> IndexBuffer::Create(uint32_t size)
	{
		switch (Renderer::InstanceAPI())
		{
		case Renderer::API::None:
			GE_CORE_ASSERT(false, "No Renderer API given!");
			break;
		case Renderer::API::OpenGL:
			return CreateRef<OpenGLIndexBuffer>(size);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
