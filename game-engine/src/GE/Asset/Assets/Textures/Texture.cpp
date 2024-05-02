#include "GE/GEpch.h"

#include "Texture.h"

#include "GE/Rendering/Renderer/Renderer.h"

#include "Platform/OpenGL/Assets/Texture/OpenGLTexture.h"

namespace GE
{
	Ref<Texture2D> Texture2D::Create(const TextureConfiguration& textureConfig, Buffer data /*= Buffer()*/)
	{
		switch (Renderer::GetAPI())
		{
		case RenderAPI::API::None:
			GE_CORE_ASSERT(false, "RendererAPI::None is not currently supported!");
			return nullptr;
			break;
		case RenderAPI::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(textureConfig, data);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}