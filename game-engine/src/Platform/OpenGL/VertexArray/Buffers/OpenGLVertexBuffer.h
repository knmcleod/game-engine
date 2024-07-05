#pragma once

#include "GE/Rendering/VertexArray/Buffers/VertexBuffer.h"

namespace GE
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
		friend class OpenGLVertexArray;
	public:
		OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(uint32_t size, float* vertices);
		~OpenGLVertexBuffer() override;

		inline Layout& GetLayout() override { return m_Layout; };
		inline void SetLayout(const Layout& layout) override { m_Layout = layout; }
	
		void SetData(const void* data, uint32_t size) override;

		void Bind() const override;
		void Unbind() const override;
	private:
		uint32_t m_RendererID;
		Layout m_Layout;
	};

}