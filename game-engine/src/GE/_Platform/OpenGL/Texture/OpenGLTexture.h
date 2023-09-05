#pragma once

#include <glad/glad.h>
#include <stb_image.h>

#include "GE/Rendering/Texture/Texture.h"

namespace GE
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path);
		virtual ~OpenGLTexture2D();

		// Inherited via Texture2D
		virtual uint32_t GetWidth() const override { return m_Width; }

		virtual uint32_t GetHeight() const override { return m_Height; }

		virtual void Bind(uint32_t slot = 0) const override;

		virtual void Unbind() const override;
	private:
		std::string m_Path;
		uint32_t m_Width, m_Height = 0;
		uint32_t m_RendererID = 0;
	};
}