#pragma once

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

		virtual void AddVertexBuffer(Ref<VertexBuffer> vertexBuffer) override;
		virtual void AddIndexBuffer(Ref<IndexBuffer> indexBuffer) override;
	
		virtual inline const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() override { return m_VertexBuffers; }
		virtual inline const Ref<IndexBuffer> GetIndexBuffer() override { return m_IndexBuffer; }
	private:
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;

		uint32_t m_RendererID = 0;
	};
}
