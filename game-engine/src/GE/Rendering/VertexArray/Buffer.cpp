#include "GE/GEpch.h"

#include "Buffer.h"
#include "GE/_Platform/OpenGL/VertexArray/OpenGLBuffer.h"
#include "GE/Rendering/RenderAPI.h"

namespace GE
{
#pragma region VertexBuffer
	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		switch (RenderAPI::GetAPI())
		{
		case RenderAPI::API::None:
			GE_CORE_ASSERT(false, "RendererAPI::None is not currently supported!");
			return nullptr;
			break;
		case RenderAPI::API::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(size);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size, float* vertices)
	{
		switch (RenderAPI::GetAPI())
		{
		case RenderAPI::API::None:
			GE_CORE_ASSERT(false, "RendererAPI::None is not currently supported!");
			return nullptr;
			break;
		case RenderAPI::API::OpenGL:
			return CreateRef<OpenGLVertexBuffer>(size, vertices);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
#pragma endregion

#pragma region IndexBuffer
	Ref<IndexBuffer> IndexBuffer::Create(uint32_t size, uint32_t* indices)
	{
		switch (RenderAPI::GetAPI())
		{
		case RenderAPI::API::None:
			GE_CORE_ASSERT(false, "RendererAPI::None is not currently supported!");
			return nullptr;
			break;
		case RenderAPI::API::OpenGL:
			return CreateRef<OpenGLIndexBuffer>(size, indices);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
#pragma endregion
}