#pragma once
#include "GE/Core/Core.h"

namespace GE
{
	enum class FramebufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8,
		RED_INTEGER,

		// Depth/stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8

	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format) : TextureFormat(format) {}
	
		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;

	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments) : Attachments(attachments) {}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	struct FramebufferSpecification
	{
		uint32_t Width = 1, Height = 1;
		uint32_t Samples = 1;
		FramebufferAttachmentSpecification AttachmentSpecification;

		bool SwapChainTarget = false; 

		// TO-DO
		//FramebufferFormat format;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual inline const FramebufferSpecification& GetSpecification() = 0;
		virtual inline void SetSpecification(const FramebufferSpecification& spec) = 0;
		
		virtual inline const uint32_t GetColorAttachmentID(uint32_t index = 0) = 0;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void Clean() = 0;
		virtual void Invalidate() = 0;
		virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;

		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};
}
