#pragma once

#include "glm/glm.hpp"

namespace GE
{
	class Framebuffer
	{
		friend class Application;
	public:
		enum class Attachment
		{
			None = 0,

			// Color
			RGBA8,
			RED_INTEGER,
			GREEN_INTEGER,
			BLUE_INTEGER,

			// Depth/stencil
			DEPTH24STENCIL8,

			// Defaults
			Depth = DEPTH24STENCIL8

		};

		struct Config
		{
			friend class Framebuffer;
			friend class OpenGLFramebuffer;
		public:
			static const uint32_t s_MaxFramebufferSize = 8192;

			Config(const Config& other)
			{
				RendererID = other.RendererID;
				Width = other.Width;
				Height = other.Height;
				Samples = other.Samples;
				SwapChainTarget = other.SwapChainTarget;
				Bounds[0] = glm::vec2(other.Bounds[0].x, other.Bounds[0].y);
				Bounds[1] = glm::vec2(other.Bounds[1].x, other.Bounds[1].y);

				for(auto& [format, id] : other.Attachments)
					Attachments[format] = id;

			}
			Config(uint32_t width, uint32_t height, const std::vector<Attachment>& attachments)
				: Width(width), Height(height)
			{
				for (auto& format : attachments)
					Attachments[format];
			}

			~Config()
			{
				Attachments.clear();
				Attachments = std::map<Attachment, uint32_t>();
			}

			void SetBounds(const glm::vec2& min, const glm::vec2& max)
			{
				if (Bounds[0] == min || Bounds[1] == max)
					return;

				if (min == glm::vec2() || max == glm::vec2()
					|| min.x > s_MaxFramebufferSize || max.x > s_MaxFramebufferSize
					|| min.y > s_MaxFramebufferSize || max.y > s_MaxFramebufferSize)
				{
					GE_CORE_WARN("Failed Framebuffer ResizeBounds.");
					return;
				}

				Bounds[0] = glm::vec2(min);
				Bounds[1] = glm::vec2(max);
			}

			void SetViewport(uint32_t width, uint32_t height)
			{
				if (Width == width || Height == height)
					return;

				if (width == 0 || height == 0
					|| width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
				{
					GE_CORE_WARN("Failed Framebuffer Resize. Size: {0}, {1}", width, height);
					return;
				}
				Width = width;
				Height = height;
			}
		
		private:
			uint32_t RendererID = 0;
			uint32_t Width = 1, Height = 1;
			// All attachments  & IDs, i.e. color & depth
			std::map<Attachment, uint32_t> Attachments = std::map<Attachment, uint32_t>();

			uint32_t Samples = 1;
			bool SwapChainTarget = false;

			glm::vec2 Bounds[2] = { {0, 0}, { 0, 0} };

		};
		
		static Ref<Framebuffer> Create(const Config& spec);

		virtual ~Framebuffer() = default;

		virtual const Config& GetConfig() const = 0;
		virtual const uint32_t& GetWidth() const = 0;
		virtual const uint32_t& GetHeight() const = 0;
		/*
		* Rturns the minimum offset of framebuffer
		*/
		virtual const glm::vec2& GetMinBounds() const = 0;
		/*
		* Returns the maximum offset of framebuffer
		*/
		virtual const glm::vec2& GetMaxBounds() const = 0;

		virtual const uint32_t& GetAttachmentID(Attachment format) = 0;

		/*
		* Sets Framebuffer::Config, Invalidates
		* @param width : viewport width
		* @param height : viewport height
		* @param min : minimum viewport bounds{ minX, minY }
		* @param max : maximum viewport bounds{ maxX, maxY }
		*/
		virtual void Resize(uint32_t width, uint32_t height, const glm::vec2& min, const glm::vec2& max) = 0;

	protected:
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Clean() = 0;
		/*
		* Sets clear color & clears buffers
		* Clears attachment[1] (RED_INTEGER) = entityID
		*/
		virtual void Clear() = 0;
		virtual void Refresh() = 0;
		/*
		* Sets internal attachment value if format is found
		*/
		virtual void ClearAttachment(Attachment format, int value) = 0;

		/*
		* Returns pixelData stored on attachment at (x,y)
		* Example: RGBA8(0), RED_INTEGER(1)
		* 
		* @param format : Attachment enum should be +1 index compared to instance Attachments
		* @param x : horizontal mouse position
		* @param y : vertical mouse position
		*/
		virtual int ReadPixel(Attachment format, int x, int y) = 0;
	};
}
