#pragma once
#include "GE/Rendering/Framebuffers/Framebuffer.h"

namespace GE
{
	class OpenGLFramebuffer : public Framebuffer
	{
		friend class Application;
	public:
		OpenGLFramebuffer(const Config& spec);
		~OpenGLFramebuffer() override;

		inline const Config& GetConfig() const override { return m_Config; }
		inline const uint32_t& GetWidth() const override { return m_Config.Width; }
		inline const uint32_t& GetHeight() const override { return m_Config.Height; }

		inline const glm::vec2& GetMinBounds() const override { return m_Config.Bounds[0]; }
		inline const glm::vec2& GetMaxBounds() const override { return m_Config.Bounds[1]; }

		inline const uint32_t& GetAttachmentID(Attachment format) override
		{ 
			GE_CORE_ASSERT(m_Config.Attachments.find(format) != m_Config.Attachments.end(), "Framebuffer Color Attachment index out of range.");
			return m_Config.Attachments.at(format);
		}

		void Resize(uint32_t width, uint32_t height, const glm::vec2& min, const glm::vec2& max) override;

	private:
		void Bind() override;
		void Unbind() override;

		void Clean() override;
		void Clear() override;
		void Refresh() override;
		void ClearAttachment(Attachment format, int value) override;
		int ReadPixel(Attachment format, int x, int y) override;
	private:
		Config m_Config;

		uint32_t m_DepthAttachmentID = 0;

	};
}