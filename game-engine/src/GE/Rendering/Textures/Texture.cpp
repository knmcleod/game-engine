#include "GE/GEpch.h"

#include "GE/Rendering/Textures/Texture.h"
#include "GE/_Platform/OpenGL/Texture/OpenGLTexture.h"

namespace GE
{
	Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height, uint32_t channels, void* data, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RenderAPI::API::None:
			GE_CORE_ASSERT(false, "RendererAPI::None is not currently supported!");
			return nullptr;
			break;
		case RenderAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(width, height, channels, data, size);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		switch (Renderer::GetAPI())
		{
		case RenderAPI::API::None:
			GE_CORE_ASSERT(false, "RendererAPI::None is not currently supported!");
			return nullptr;
			break;
		case RenderAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(path);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}