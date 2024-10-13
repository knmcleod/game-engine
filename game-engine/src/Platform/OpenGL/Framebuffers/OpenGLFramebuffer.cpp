#include "GE/GEpch.h"

#include "OpenGLFramebuffer.h"

#include "GE/Rendering/Renderer/Renderer.h"

#include <glad/glad.h>

namespace GE
{
	static bool IsDepthFormat(Framebuffer::Attachment format)
	{
		switch (format)
		{
		case Framebuffer::Attachment::DEPTH24STENCIL8:
			return true;
			break;
		default:
			break;
		}
		return false;
	}

	static GLenum TextureTarget(bool isMultiSample)
	{
		return isMultiSample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	}

	static GLenum FBFormatToGL(Framebuffer::Attachment format)
	{
		switch (format)
		{
		case Framebuffer::Attachment::RGBA8:
			return GL_RGBA8;
			break;
		case Framebuffer::Attachment::RED_INTEGER:
			return GL_RED_INTEGER;
			break;
		case Framebuffer::Attachment::GREEN_INTEGER:
			return GL_GREEN_INTEGER;
			break;
		case Framebuffer::Attachment::BLUE_INTEGER:
			return GL_BLUE_INTEGER;
			break;
		case Framebuffer::Attachment::DEPTH24STENCIL8:
			return GL_DEPTH24_STENCIL8;
			break;
		default:
			GE_CORE_ASSERT(false, "Unsupported Framebuffer::Attachment.");
			break;
		}
		return 0;
	}

	static void CreateTextures(bool isMultiSample, uint32_t* outID, uint32_t count)
	{
		glCreateTextures(TextureTarget(isMultiSample), count, outID);
		
	}

	static void BindTexture(bool isMultiSample, uint32_t id)
	{
		glBindTexture(TextureTarget(isMultiSample), id);
	}

	static void AttachColorTexture(GLenum internalFormat, GLenum format, uint32_t id, 
		int samples, uint32_t width, uint32_t height, int index)
	{
		bool isMultisample = samples > 1;
		if (isMultisample)
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		}

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(isMultisample), id, 0);

	}
	
	static void AttachDepthTexture(GLenum format, GLenum attachmentType, uint32_t id,
		int samples, uint32_t width, uint32_t height)
	{
		bool isMultisample = samples > 1;
		if (isMultisample)
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
		else
		{
			glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		}
		
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(isMultisample), id, 0);

	}

	OpenGLFramebuffer::OpenGLFramebuffer(const Framebuffer::Config& spec) : m_Config(spec)
	{
		Refresh();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		Clean();
	}

	void OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_Config.RendererID);
	}

	void OpenGLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height, const glm::vec2& min, const glm::vec2& max)
	{	
		m_Config.SetViewport(width, height);
		m_Config.SetBounds(min, max);

		Refresh();
	}

	void OpenGLFramebuffer::Clean()
	{
		glDeleteFramebuffers(1, &m_Config.RendererID);
		for (auto& [format, id] : m_Config.Attachments)
		{
			glDeleteTextures((GLsizei)1, &id);
			id = 0;
		}
		m_DepthAttachmentID = 0;
	}

	void OpenGLFramebuffer::Clear()
	{
		Renderer::ClearColor({ 0.25f, 0.25f, 0.25f, 1.0f });
		Renderer::CleanBuffers();

		ClearAttachment(Attachment::RED_INTEGER, -1);
	}

	void OpenGLFramebuffer::Refresh()
	{
		if (m_Config.RendererID)
		{
			Clean();
		}

		glCreateFramebuffers(1, &m_Config.RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_Config.RendererID);

		// Attachments 
		bool multisample = m_Config.Samples > 1;
		for (auto& [format, id] : m_Config.Attachments)
		{
			CreateTextures(multisample, &id, 1);
			BindTexture(multisample, id);

			switch (format)
			{
			case Attachment::RGBA8:
				AttachColorTexture(GL_RGBA8, GL_RGBA, id, m_Config.Samples, m_Config.Width, m_Config.Height, 0);
				break;
			case Attachment::RED_INTEGER:
				AttachColorTexture(GL_R32I, GL_RED_INTEGER, id, m_Config.Samples, m_Config.Width, m_Config.Height, 1);
				break;
			case Attachment::GREEN_INTEGER:
				AttachColorTexture(GL_R32I, GL_GREEN_INTEGER, id, m_Config.Samples, m_Config.Width, m_Config.Height, 2);
				break;
			case Attachment::BLUE_INTEGER:
				AttachColorTexture(GL_R32I, GL_BLUE_INTEGER, id, m_Config.Samples, m_Config.Width, m_Config.Height, 3);
				break;
			case Attachment::DEPTH24STENCIL8:
				AttachDepthTexture(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, id, m_Config.Samples, m_Config.Width, m_Config.Height);
				m_DepthAttachmentID = id;
				break;
			default:
				GE_ERROR("Unsupported Framebuffer Attachment");
				break;
			}
		}

		uint64_t count = m_Config.Attachments.size() - (m_DepthAttachmentID != 0 ? 1 : 0);
		if (count > 1)
		{
			GE_CORE_ASSERT(count <= 4, "Four(4) or more color attachments present.");
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
									GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers((GLsizei)count, buffers);
		}
		else if (m_DepthAttachmentID != 0 && count == 1) // Depth-pass only
		{
			glDrawBuffer(GL_NONE);
		}

		GE_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer Status Incomplete!");

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		Clear();
	}

	void OpenGLFramebuffer::ClearAttachment(Attachment format, int value)
	{
		GE_CORE_ASSERT(m_Config.Attachments.find(format) != m_Config.Attachments.end(),
			"Framebuffer Attachment Index is out of range.");

		glClearTexImage(m_Config.Attachments.at(format), 0,
			FBFormatToGL(format), GL_INT, &value);
		
	}

	int OpenGLFramebuffer::ReadPixel(Attachment format, int x, int y)
	{
		this->Bind();
		GE_CORE_ASSERT(m_Config.Attachments.find(format) != m_Config.Attachments.end(),
			"Framebuffer Attachment Index is out of range.");
		GLenum formatIndex = (uint32_t)format - 1;
		glReadBuffer(GL_COLOR_ATTACHMENT0 + formatIndex);
		int pixelData = -1;
		glReadPixels(x, y, 1, 1, FBFormatToGL(format), GL_INT, &pixelData);
		return pixelData;
	}

}