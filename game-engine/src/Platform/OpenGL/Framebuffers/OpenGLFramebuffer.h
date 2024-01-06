#pragma once
#include "GE/Rendering/Framebuffers/Framebuffer.h"

namespace GE
{
	class OpenGLFramebuffer : public Framebuffer
	{
	private:
		uint32_t m_RendererID = 0;

		std::vector<uint32_t> m_ColorAttachmentsID;
		uint32_t m_DepthAttachmentID = 0;

		FramebufferSpecification m_Spec;
		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
		FramebufferTextureSpecification m_DepthAttachmentSpec;

	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		virtual inline const FramebufferSpecification& GetSpecification() override { return m_Spec; }
		virtual inline void SetSpecification(const FramebufferSpecification& spec) override { m_Spec = spec; }

		virtual inline const uint32_t GetColorAttachmentID(uint32_t index = 0) override
		{ 
			GE_CORE_ASSERT(index < m_ColorAttachmentsID.size(), "Framebuffer Color Attachment index out of range."); 
			return m_ColorAttachmentsID[index];
		}

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void Clean() override;
		virtual void Invalidate() override;
		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

	};
}