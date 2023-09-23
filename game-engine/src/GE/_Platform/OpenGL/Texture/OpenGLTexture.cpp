#include "GE/GEpch.h"

#include "OpenGLTexture.h"

namespace GE
{
	OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, uint32_t channels, void* data, uint32_t size) : m_Width(width), m_Height(height)
	{
		if (channels == 4)
		{
			m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGBA;
		}
		else if (channels == 3)
		{
			m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGB;
		}
		else
		{
			m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGBA;
		}

		GE_CORE_ASSERT(m_InternalFormat && m_DataFormat, "Format not supported!");

		glGenTextures(1, &m_RendererID);
		this->Bind();

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		//glTextureStorage2D(m_RendererID, 0, m_InternalFormat, m_Width, m_Height);

		uint32_t bpp = (m_DataFormat == GL_RGBA ? 4 : 3);
		GE_CORE_ASSERT(size == m_Width * m_Height * bpp, "Texture size incorrect! Data must be the entire texture!");

		glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path): m_Path(path)
	{
		int width = 0, height = 0, channels = 4;
		stbi_set_flip_vertically_on_load(1);
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		GE_CORE_ASSERT(data, "Failed to load image!");
		
		m_Width = width;
		m_Height = height;

		if (channels == 4)
		{
			m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGBA;
		}
		else if(channels == 3)
		{
			m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGB;
		}
		else
		{
			m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGBA;
		}

		GE_CORE_ASSERT(m_InternalFormat && m_DataFormat, "Format not supported!");

		glGenTextures(1, &m_RendererID);
		this->Bind();

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}
	
	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		uint32_t bpp = (m_DataFormat == GL_RGBA ? 4 : 3);
		GE_CORE_ASSERT(size == m_Width * m_Height * bpp, "Texture size incorrect! Data must be the entire texture!");

		//glTexImage2D(m_RendererID, 0, m_InternalFormat, m_Width, m_Height, 0, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}

	void OpenGLTexture2D::Unbind() const
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}