#include "GE/GEpch.h"

#include "Framebuffer.h"

#include "GE/Rendering/Renderer/Renderer.h"

#include "Platform/OpenGL/Framebuffers/OpenGLFramebuffer.h"

namespace GE
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
		case RenderAPI::API::None:
			GE_CORE_ASSERT(false, "RendererAPI::None is not currently supported!");
			return nullptr;
			break;
		case RenderAPI::API::OpenGL:
			return CreateRef<OpenGLFramebuffer>(spec);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}