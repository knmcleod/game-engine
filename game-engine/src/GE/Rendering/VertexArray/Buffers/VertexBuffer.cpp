#include "GE/GEpch.h"

#include "VertexBuffer.h"

#include "GE/Rendering/Renderer/Renderer.h"

#include "Platform/OpenGL/VertexArray/Buffers/OpenGLVertexBuffer.h"

namespace GE
{
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (Renderer::InstanceAPI())
		{
		case Renderer::API::None:
			GE_CORE_ASSERT(false, "No Renderer API given!");
			return nullptr;
			break;
		case Renderer::API::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(size);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size, float* vertices)
	{
		switch (Renderer::InstanceAPI())
		{
		case Renderer::API::None:
			GE_CORE_ASSERT(false, "No Renderer API given!");
			return nullptr;
			break;
		case Renderer::API::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(size, vertices);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}