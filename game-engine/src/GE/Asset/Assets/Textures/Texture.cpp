#include "GE/GEpch.h"

#include "Texture.h"

#include "GE/Rendering/Renderer/Renderer.h"

#include "Platform/OpenGL/Assets/OpenGLTexture.h"

namespace GE
{
	Ref<Texture2D> Texture2D::Create(UUID handle, const Config& textureConfig, Buffer data /*= Buffer()*/)
	{
		switch (Renderer::Get()->GetAPI())
		{
		case Renderer::API::None:
			GE_CORE_ASSERT(false, "No Renderer API given!");
			return nullptr;
			break;
		case Renderer::API::OpenGL:
			return CreateRef<OpenGLTexture2D>(handle, textureConfig, data);
			break;
		}
		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}