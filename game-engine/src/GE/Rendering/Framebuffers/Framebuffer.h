#pragma once

namespace GE
{
	class Framebuffer
	{
	public:
		enum class TextureFormat
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

		struct TextureSpecification
		{
			TextureSpecification() = default;
			TextureSpecification(TextureFormat format) : TextureFormat(format) {}

			TextureFormat TextureFormat = TextureFormat::None;
		};

		struct AttachmentSpecification
		{
			AttachmentSpecification() = default;
			AttachmentSpecification(std::initializer_list<TextureSpecification> attachments) : Attachments(attachments) {}

			std::vector<TextureSpecification> Attachments;
		};

		struct Config
		{
			uint32_t Width = 1, Height = 1;
			uint32_t Samples = 1;
			AttachmentSpecification AttachmentSpecification;

			bool SwapChainTarget = false;
		};
		
		static Ref<Framebuffer> Create(const Config& spec);

		virtual ~Framebuffer() = default;

		virtual const Config& GetConfig() = 0;
		virtual const uint32_t& GetWidth() = 0;
		virtual const uint32_t& GetHeight() = 0;

		virtual const uint32_t GetColorAttachmentID(uint32_t index = 0) = 0;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void Clean() = 0;
		virtual void Invalidate() = 0;
		virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;

		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

	};
}
