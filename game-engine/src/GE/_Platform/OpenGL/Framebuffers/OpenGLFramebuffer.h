#pragma once
#include "GE/Rendering/Framebuffers/Framebuffer.h"

namespace GE
{
	class OpenGLFramebuffer : public Framebuffer
	{
	private:
		uint32_t m_RendererID;
		uint32_t m_ColorAttachment, m_DepthAttachment;
		FramebufferSpecification m_Spec;

	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer();

		virtual inline const FramebufferSpecification& GetSpecification() override { return m_Spec; }
		virtual inline void SetSpecification(const FramebufferSpecification& spec) override { m_Spec = spec; }

		virtual inline const uint32_t GetColorAttachment() override { return m_ColorAttachment; }

		virtual void Bind() override;
		virtual void Unbind() override;

		void Invalidate();

	};
}