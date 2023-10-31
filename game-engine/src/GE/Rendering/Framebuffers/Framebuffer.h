#pragma once
#include "GE/Core/Core.h"

namespace GE
{
	struct FramebufferSpecification
	{
		uint32_t Width, Height;
		uint32_t Samples;

		bool SwapChainTarget = false; 

		// TO-DO
		//FramebufferFormat format;
	};

	class Framebuffer
	{
	protected:

	public:
		virtual inline const FramebufferSpecification& GetSpecification() = 0;
		virtual inline void SetSpecification(const FramebufferSpecification& spec) = 0;
		
		virtual inline const uint32_t GetColorAttachment() = 0;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};
}
