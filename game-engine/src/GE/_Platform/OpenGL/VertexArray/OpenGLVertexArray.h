#pragma once

#include <glad/glad.h>

#include "GE/Rendering/VertexArray/VertexArray.h"

namespace GE
{
	class OpenGLVertexArray : public VertexArray
	{
	public:
		// Created on construct using OpenGL
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) override;
		virtual void AddIndexBuffer(const Ref<IndexBuffer>& indexBuffer) override;
	
		virtual inline std::vector<Ref<VertexBuffer>> const& GetVertexBuffers() override { return m_VertexBuffers; }
		virtual inline Ref<IndexBuffer> const& GetIndexBuffer() override { return m_IndexBuffer; }
	private:
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;

		uint32_t m_RendererID = 0;
	};
}
