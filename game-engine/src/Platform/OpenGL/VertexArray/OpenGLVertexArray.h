#pragma once

#include "GE/Rendering/VertexArray/VertexArray.h"

namespace GE
{
	class OpenGLVertexArray : public VertexArray
	{
	public:
		OpenGLVertexArray();
		~OpenGLVertexArray() override;

		void Bind() const override;
		void Unbind() const override;

		void AddVertexBuffer(Ref<VertexBuffer> vertexBuffer) override;
		void AddIndexBuffer(Ref<IndexBuffer> indexBuffer) override;
		
		inline const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() override { return m_VertexBuffers; }
		inline const Ref<IndexBuffer> GetIndexBuffer() override { return m_IndexBuffer; }
	private:
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;

		uint32_t m_RendererID = 0;
	};
}
