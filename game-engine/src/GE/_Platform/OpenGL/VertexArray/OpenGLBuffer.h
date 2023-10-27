#pragma once

#include <glad/glad.h>

#include "GE/Rendering/VertexArray/Buffer.h"


namespace GE
{
	class OpenGLVertexBuffer : public VertexBuffer
	{
	public:
		OpenGLVertexBuffer(uint32_t size);
		OpenGLVertexBuffer(uint32_t size, float* vertices);
		virtual ~OpenGLVertexBuffer();

		virtual inline BufferLayout& GetLayout() override { return m_Layout; };
		virtual inline void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }
	
		virtual void SetData(const void* data, uint32_t size) override;

		// Binds buffer using OpenGL
		virtual void Bind() const override;
		// Unbinds buffer using OpenGL
		virtual void Unbind() const override;
	private:
		uint32_t m_RendererID;
		BufferLayout m_Layout;
	};

	class OpenGLIndexBuffer : public IndexBuffer
	{
	public:
		OpenGLIndexBuffer(uint32_t size, uint32_t* indices);
		virtual ~OpenGLIndexBuffer();

		virtual uint32_t GetCount() const { return m_Count; }

		// Binds buffer using OpenGL
		virtual void Bind() const override;
		// Unbinds buffer using OpenGL
		virtual void Unbind() const override;
	private:
		uint32_t m_RendererID;
		uint32_t m_Count;
	};

}