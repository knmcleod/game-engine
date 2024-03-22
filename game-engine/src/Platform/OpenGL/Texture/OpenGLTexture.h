#pragma once

#include "GE/Rendering/Textures/Texture.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace GE
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		virtual bool operator==(const Texture& other) const override { return this->GetID() == ((Texture&)other).GetID(); };

		OpenGLTexture2D(const TextureConfiguration& textureConfig);
		OpenGLTexture2D(const std::string& path);
		virtual ~OpenGLTexture2D();

		virtual TextureConfiguration GetConfig() const override { return m_Config; }
		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		virtual uint32_t GetID() const override { return m_RendererID; }
		virtual const std::string& GetProjectPath() const override { return m_Path; }

		virtual void SetData(void* data, uint32_t size) override;
		virtual void SetFormats(int channels) override;
		virtual void Bind(uint32_t slot = 0) const override;
		virtual void Unbind() const override;
	private:
		//GLenum m_InternalFormat;
		//GLenum m_DataFormat;
		
		uint32_t m_RendererID = 0;
		uint32_t m_Width, m_Height = 0;
		std::string m_Path;

		TextureConfiguration m_Config;
	};
}