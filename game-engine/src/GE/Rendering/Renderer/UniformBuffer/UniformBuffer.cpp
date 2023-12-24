#include "GE/GEpch.h"
#include "UniformBuffer.h"

#include "GE/Rendering/Renderer/Renderer.h"
#include "GE/_Platform/OpenGL/UniformBuffer/OpenGLUniformBuffer.h"

namespace GE
{
	static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding)
	{
		switch (Renderer::GetAPI())
		{
		case RenderAPI::API::None: 
		{
			GE_CORE_ASSERT(false, "Unsupported RenderAPI!")
			return nullptr;
		}
		case RenderAPI::API::OpenGL:
		{
			return CreateRef<OpenGLUniformBuffer>(size, binding);
		}
		}
		GE_CORE_ASSERT(false, "Unsupported RenderAPI!")
		return nullptr;
	}
}
