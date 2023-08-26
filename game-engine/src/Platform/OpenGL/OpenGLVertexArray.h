#pragma once

#include "GE/Renderer/VertexArray.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <glad/glad.h>

namespace GE
{
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		virtual ~OpenGLVertexArray();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer) override;
		virtual void AddIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer) override;
	
		virtual std::vector<std::shared_ptr<VertexBuffer>> const& GetVertexBuffers() override { return m_VertexBuffers; }
		virtual std::vector<std::shared_ptr<IndexBuffer>> const& GetIndexBuffers() override { return m_IndexBuffers; }
	private:
		std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
		std::vector<std::shared_ptr<IndexBuffer>> m_IndexBuffers;

		uint32_t m_RendererID = 0;
	};
}
