#include "GE/GEpch.h"

#include "Framebuffer.h"

#include "GE/Rendering/Renderer/Renderer.h"

#include "Platform/OpenGL/Framebuffers/OpenGLFramebuffer.h"

namespace GE
{
	Ref<Framebuffer> Framebuffer::Create(const Framebuffer::Config& spec)
	{
		switch (Renderer::InstanceAPI())
		{
		case Renderer::API::None:
			GE_CORE_ASSERT(false, "No Renderer API given!");
			return nullptr;
			break;
		case Renderer::API::OpenGL:
			return CreateRef<OpenGLFramebuffer>(spec);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}