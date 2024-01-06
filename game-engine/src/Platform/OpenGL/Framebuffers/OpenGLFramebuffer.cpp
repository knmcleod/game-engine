#include "GE/GEpch.h"

#include "OpenGLFramebuffer.h"

#include <glad/glad.h>

namespace GE
{
	static const uint32_t s_MaxFramebufferSize = 8192;

	static bool IsDepthFormat(FramebufferTextureFormat format)
	{
		switch (format)
		{
		case FramebufferTextureFormat::DEPTH24STENCIL8:
			return true;
			break;
		default:
			return false;
			break;
		}
	}

	static GLenum TextureTarget(bool isMultisample)
	{
		return isMultisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	}

	static GLenum FBTextureFormatToGL(FramebufferTextureFormat format)
	{
		switch (format)
		{
		case GE::FramebufferTextureFormat::RGBA8:
			return GL_RGBA8;
			break;
		case GE::FramebufferTextureFormat::RED_INTEGER:
			return GL_RED_INTEGER;
			break;
		default:
			GE_CORE_ASSERT(false, "Unsupported Framebuffer Texture Format.");
			return 0;
			break;
		}
	}

	static void CreateTextures(bool isMultisample, uint32_t* outID, uint32_t count)
	{
		glCreateTextures(TextureTarget(isMultisample), count, outID);
		
	}

	static void BindTexture(bool isMultisample, uint32_t id)
	{
		glBindTexture(TextureTarget(isMultisample), id);
	}

	static void AttachColorTexture(GLenum internalFormat, GLenum format, uint32_t id, int samples, uint32_t width, uint32_t height, int index)
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

	OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec) : m_Spec(spec)
	{
		for (auto format : m_Spec.AttachmentSpecification.Attachments)
		{
			if (!IsDepthFormat(format.TextureFormat))
				m_ColorAttachmentSpecs.emplace_back(format);
			else
				m_DepthAttachmentSpec = format;
		}

		Invalidate();
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		Clean();
	}

	void OpenGLFramebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Spec.Width, m_Spec.Height);

	}

	void OpenGLFramebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
	{	
		if (width == 0 || height == 0
			|| width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
		{
			GE_CORE_WARN("Attempted Framebuffer Resize. Size: {0}, {1}", width, height);
			return;
		}

		m_Spec.Width = width;
		m_Spec.Height = height;
		Invalidate();
	}

	void OpenGLFramebuffer::Clean()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(m_ColorAttachmentsID.size(), m_ColorAttachmentsID.data());
		glDeleteTextures(1, &m_DepthAttachmentID);

		m_ColorAttachmentsID.clear();
		m_DepthAttachmentID = 0;
	}

	void OpenGLFramebuffer::Invalidate()
	{
		if (m_RendererID)
		{
			Clean();
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		// Attachments 
		bool multisample = m_Spec.Samples > 1;
		if (m_ColorAttachmentSpecs.size())
		{
			m_ColorAttachmentsID.resize(m_ColorAttachmentSpecs.size());
			CreateTextures(multisample, m_ColorAttachmentsID.data(), m_ColorAttachmentsID.size());

			for (size_t i = 0; i < m_ColorAttachmentsID.size(); i++)
			{
				BindTexture(multisample, m_ColorAttachmentsID[i]);
				switch (m_ColorAttachmentSpecs[i].TextureFormat)
				{
				case FramebufferTextureFormat::RGBA8:
					AttachColorTexture(GL_RGBA8, GL_RGBA, m_ColorAttachmentsID[i], m_Spec.Samples, m_Spec.Width, m_Spec.Height, i);
					break;
				case FramebufferTextureFormat::RED_INTEGER:
					AttachColorTexture(GL_R32I, GL_RED_INTEGER, m_ColorAttachmentsID[i], m_Spec.Samples, m_Spec.Width, m_Spec.Height, i);
					break;
				default:
					GE_ERROR("Unsupported Texture Format");
					break;
				}
			}
		}

		if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None)
		{
			CreateTextures(multisample, &m_DepthAttachmentID, 1);
			BindTexture(multisample, m_DepthAttachmentID);
			switch (m_DepthAttachmentSpec.TextureFormat)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:
				AttachDepthTexture(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_DepthAttachmentID, m_Spec.Samples, m_Spec.Width, m_Spec.Height);
				break;
			}
		}

		if (m_ColorAttachmentsID.size() > 1)
		{
			GE_CORE_ASSERT(m_ColorAttachmentsID.size() <= 4, "Four(4) or more color attachments present.");
			GLenum buffers[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
									GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
			glDrawBuffers(m_ColorAttachmentsID.size(), buffers);
		}
		else if (m_ColorAttachmentsID.empty()) // Depth-pass only
		{
			glDrawBuffer(GL_NONE);
		}

		GE_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer Status Incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
		GE_CORE_ASSERT(attachmentIndex < m_ColorAttachmentsID.size(),
			"Framebuffer Attachment Index is out of range.");
		auto& spec = m_ColorAttachmentSpecs[attachmentIndex];
		
		glClearTexImage(m_ColorAttachmentsID[attachmentIndex], 0,
			FBTextureFormatToGL(spec.TextureFormat), GL_INT, &value);
		
	}

	int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		GE_CORE_ASSERT(attachmentIndex < m_ColorAttachmentsID.size(),
			"Framebuffer Attachment Index is out of range.");
		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		int pixelData;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
		return pixelData;
	}

}