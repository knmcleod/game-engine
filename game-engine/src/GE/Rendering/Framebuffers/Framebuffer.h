#pragma once
#include "GE/Core/Core.h"

namespace GE
{
	struct FramebufferSpecification
	{
		uint32_t Width = 1, Height = 1;
		uint32_t Samples;

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
		
		virtual inline const uint32_t GetColorAttachment() = 0;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual void Clean() = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};
}
