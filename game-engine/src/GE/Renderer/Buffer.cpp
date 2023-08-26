#include "GEpch.h"
#include "GEpch.h"

#include "Buffer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace GE
{
	VertexBuffer* VertexBuffer::Create(uint32_t size, float* vertices)
	{
		switch (Renderer::GetAPI())
		{
		case Renderer::RendererAPI::None:
			GE_CORE_ASSERT(false, "RendererAPI::None is not currently supported!");
			return nullptr;
			break;
		case Renderer::RendererAPI::OpenGL:
			return new OpenGLVertexBuffer(size, vertices);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t size, uint32_t* indices)
	{
		switch (Renderer::GetAPI())
		{
		case Renderer::RendererAPI::None:
			GE_CORE_ASSERT(false, "RendererAPI::None is not currently supported!");
			return nullptr;
			break;
		case Renderer::RendererAPI::OpenGL:
			return new OpenGLIndexBuffer(size, indices);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}