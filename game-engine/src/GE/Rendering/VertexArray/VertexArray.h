#pragma once

#include <memory>

#include "Buffer.h"

namespace GE
{
	class VertexArray
	{
	public:
		virtual ~VertexArray() {}

		// Binds Vertex Array
		virtual void Bind() const = 0;

		// Unbinds Vertex Array
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer) = 0;
		virtual void AddIndexBuffer(const Ref<IndexBuffer>& indexBuffer) = 0;

		virtual inline const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() { return m_VertexBuffers; };
		virtual inline const Ref<IndexBuffer>& GetIndexBuffer() { return m_IndexBuffer; };

		static Ref<VertexArray> Create();

	private:
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;

		uint32_t m_RendererID = 0;
	};
}
