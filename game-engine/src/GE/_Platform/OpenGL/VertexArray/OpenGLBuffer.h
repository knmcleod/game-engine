#pragma once

#include <glad/glad.h>

#include "GE/Rendering/VertexArray/Buffer.h"


namespace GE
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size, float* vertices);
		virtual ~OpenGLVertexBuffer();

		// Binds buffer using OpenGL
		virtual void Bind() const override;

		// Unbinds buffer using OpenGL
		virtual void Unbind() const override;

		virtual inline BufferLayout& GetLayout() override { return m_Layout; };
		virtual inline void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t size, uint32_t* indices);
		virtual ~OpenGLIndexBuffer();

		// Binds buffer using OpenGL
		virtual void Bind() const override;

		// Unbinds buffer using OpenGL
		virtual void Unbind() const override;

		virtual uint32_t GetCount() const { return m_Count; }
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};

}