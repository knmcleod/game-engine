#pragma once

#include "GE/Renderer/Buffer.h"

#include <memory>

namespace GE
{
	class VertexArray
	{
	public:
		virtual ~VertexArray() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(std::shared_ptr<VertexBuffer> const& vertexBuffer) = 0;
		virtual void AddIndexBuffer(std::shared_ptr<IndexBuffer> const& indexBuffer) = 0;

		virtual std::vector<std::shared_ptr<VertexBuffer>> const& GetVertexBuffers() = 0;
		virtual std::vector<std::shared_ptr<IndexBuffer>> const& GetIndexBuffers() = 0;

		static VertexArray* Create();
	};
}
