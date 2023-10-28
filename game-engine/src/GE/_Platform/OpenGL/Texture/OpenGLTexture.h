#pragma once

#include <glad/glad.h>
#include <stb_image.h>

#include "GE/Rendering/Textures/Texture.h"

namespace GE
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		virtual bool operator==(const Texture& other) const override { return this->GetID() == ((Texture&)other).GetID(); };

		OpenGLTexture2D(uint32_t width, uint32_t height, uint32_t channels, void* data = NULL, uint32_t size = 0);
		OpenGLTexture2D(const std::string& path);
		virtual ~OpenGLTexture2D();

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }

		virtual uint32_t GetID() const override { return m_RendererID; }

		virtual void SetData(void* data, uint32_t size) override;

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void Unbind() const override;
	private:
		GLenum m_InternalFormat;
		GLenum m_DataFormat;

		uint32_t m_RendererID = 0;
		uint32_t m_Width, m_Height = 0;
		std::string m_Path;
	};
}