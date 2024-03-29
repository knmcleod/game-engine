#include "GE/GEpch.h"

#include "OpenGLTexture.h"

namespace GE
{
	static GLenum GLInternalFormatFromImageFormat(const ImageFormat& format)
	{
		switch (format)
		{
		case ImageFormat::R8:
			return GL_RGB8;
		case ImageFormat::RBG8:
			return GL_RGB8;
		case ImageFormat::RBGA8:
			return GL_RGB8;
		case ImageFormat::RGBA32F:
			return GL_RGB8;
		case ImageFormat::None:
			return NULL;
			break;
		}
	}

	static GLenum GLDataFormatFromDataFormat(const DataFormat& format)
	{
		switch (format)
		{
		case DataFormat::RGB:
			return GL_RGB; // 3 channels
		case DataFormat::RGBA:
			return GL_RGBA; // 4 channels
		case DataFormat::None:
			return NULL;
			break;
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(const TextureConfiguration& textureConfig)
	{
		GE_CORE_ASSERT(textureConfig.InternalFormat != ImageFormat::None, "No Internal Format Specified.");

		m_Config.Width = textureConfig.Width;
		m_Config.Height = textureConfig.Height;
		m_Config.GenerateMips = textureConfig.GenerateMips;
		m_Config.InternalFormat = textureConfig.InternalFormat;
		m_Config.Format = textureConfig.Format;

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTextureStorage2D(GL_TEXTURE_2D, 1,
			GLInternalFormatFromImageFormat(m_Config.InternalFormat), m_Config.Width, m_Config.Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path): m_Path(path)
	{
		int width = 0, height = 0, channels = 4;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = nullptr;
		{
			GE_PROFILE_SCOPE("stbi_load - OpenGLTexture2D::OpenGLTexture2D(const std::string&)");
			data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		}
		GE_CORE_ASSERT(data, "Failed to load image!");
		
		m_Config.Height = height;
		m_Config.Width = width;
		SetFormats(channels);

		GE_CORE_ASSERT(m_Config.InternalFormat != ImageFormat::None, "No Internal Format Specified.");

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glTextureStorage2D(GL_TEXTURE_2D, 1,
			GLInternalFormatFromImageFormat(m_Config.InternalFormat), m_Config.Width, m_Config.Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0,
			GLInternalFormatFromImageFormat(m_Config.InternalFormat),
			m_Config.Width, m_Config.Height, 0,
			GLDataFormatFromDataFormat(m_Config.Format), GL_UNSIGNED_BYTE, data);

		if(m_Config.GenerateMips)
			glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}
	
	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		GE_PROFILE_FUNCTION();

		uint32_t bpp = (GLDataFormatFromDataFormat(m_Config.Format) == GL_RGBA ? 4 : 3);
		GE_CORE_ASSERT(size == m_Config.Width * m_Config.Height * bpp, "Texture size incorrect! Data must be the entire texture!");

		glTexImage2D(GL_TEXTURE_2D, 0,
			GLInternalFormatFromImageFormat(m_Config.InternalFormat),
			m_Config.Width, m_Config.Height, 0,
			GLDataFormatFromDataFormat(m_Config.Format), GL_UNSIGNED_BYTE, data);
		
		if (m_Config.GenerateMips)
			glGenerateMipmap(GL_TEXTURE_2D);
	}

	void OpenGLTexture2D::SetFormats(int channels)
	{
		if (channels == 3)
		{
			m_Config.InternalFormat = ImageFormat::RBG8;
			m_Config.Format = DataFormat::RGB;
		}
		else if (channels == 4)
		{
			m_Config.InternalFormat = ImageFormat::RBGA8;
			m_Config.Format = DataFormat::RGBA;
		}
		else
			GE_CORE_ERROR("Failed to set Texture Configuration Formats.");
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