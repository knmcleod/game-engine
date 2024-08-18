#include "GE/GEpch.h"

#include "OpenGLTexture.h"

#include <glad/glad.h>

namespace GE
{
	static GLenum GLInternalFormatFromImageFormat(const Texture::ImageFormat& format)
	{
		switch (format)
		{
		case Texture::ImageFormat::R8:
			return GL_RGB8;
		case Texture::ImageFormat::RGB8:
			return GL_RGB8;
		case Texture::ImageFormat::RGBA8:
			return GL_RGB8;
		case Texture::ImageFormat::RGBA32F:
			return GL_RGB8;
		case Texture::ImageFormat::None:
			return NULL;
			break;
		}
		return NULL;
	}

	static GLenum GLDataFormatFromDataFormat(const Texture::DataFormat& format)
	{
		switch (format)
		{
		case Texture::DataFormat::RGB:
			return GL_RGB; // 3 channels
		case Texture::DataFormat::RGBA:
			return GL_RGBA; // 4 channels
		case Texture::DataFormat::None:
			return NULL;
			break;
		}
		return NULL;
	}

	OpenGLTexture2D::OpenGLTexture2D(const Config& textureConfig, Buffer data /* = Buffer() */)
	{
		GE_CORE_ASSERT(textureConfig.InternalFormat != ImageFormat::None, "No Internal Format Specified.");

		p_Type = Asset:: Type::Texture2D;
		m_Config.Name = textureConfig.Name;
		m_Config.Width = textureConfig.Width;
		m_Config.Height = textureConfig.Height;
		m_Config.GenerateMips = textureConfig.GenerateMips;
		m_Config.InternalFormat = textureConfig.InternalFormat;
		m_Config.Format = textureConfig.Format;

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTextureStorage2D(GL_TEXTURE_2D, 1,
			GLInternalFormatFromImageFormat(m_Config.InternalFormat), m_Config.Width, m_Config.Height);

		if (data)
			SetData(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}
	
	Ref<Asset> OpenGLTexture2D::GetCopy()
	{
		GE_CORE_WARN("Could not copy OpenGLTexture2D Asset. Returning nullptr.");
		return nullptr;
	}

	void OpenGLTexture2D::SetData(Buffer data)
	{
		GE_PROFILE_FUNCTION();
		this->m_TextureBuffer = data;

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		uint32_t bpp = (GLDataFormatFromDataFormat(m_Config.Format) == GL_RGBA ? 4 : 3);
		GE_CORE_ASSERT(data.Size == m_Config.Width * m_Config.Height * bpp, "Texture size incorrect! Data must be the entire texture!");

		glTexImage2D(GL_TEXTURE_2D, 0,
			GLInternalFormatFromImageFormat(m_Config.InternalFormat),
			m_Config.Width, m_Config.Height, 0,
			GLDataFormatFromDataFormat(m_Config.Format), GL_UNSIGNED_BYTE, data.Data);
		
		if (m_Config.GenerateMips)
			glGenerateMipmap(GL_TEXTURE_2D);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		GE_PROFILE_FUNCTION();

		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTextureUnit(slot, m_RendererID);
	}

	void OpenGLTexture2D::Unbind() const
	{
		GE_PROFILE_FUNCTION();

		glBindTexture(GL_TEXTURE_2D, 0);
	}
}