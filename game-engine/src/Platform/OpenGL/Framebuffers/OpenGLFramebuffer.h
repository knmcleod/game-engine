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

		Config p_Config;
		std::vector<TextureSpecification> m_ColorAttachmentSpecs;
		TextureSpecification m_DepthAttachmentSpec;

	public:
		OpenGLFramebuffer(const Config& spec);
		~OpenGLFramebuffer();

		inline const Config& GetConfig() override { return p_Config; }
		inline const uint32_t& GetWidth() override { return p_Config.Width; }
		inline const uint32_t& GetHeight() override { return p_Config.Height; }

		inline const uint32_t GetColorAttachmentID(uint32_t index = 0) override
		{ 
			GE_CORE_ASSERT(index < m_ColorAttachmentsID.size(), "Framebuffer Color Attachment index out of range."); 
			return m_ColorAttachmentsID[index];
		}

		void Bind() override;
		void Unbind() override;

		void Resize(uint32_t width, uint32_t height) override;
		void Clean() override;
		void Invalidate() override;
		void ClearAttachment(uint32_t attachmentIndex, int value) override;

		int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

	};
}