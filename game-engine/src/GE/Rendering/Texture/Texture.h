#pragma once

#include <string>

#include "GE/Core/Core.h"

#include "GE/Rendering/Renderer/Renderer.h"

namespace GE
{
	class Texture
	{
	public:
		virtual ~Texture() = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void Unbind() const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		virtual ~Texture2D() = 0;

		static Texture2D* Create(const std::string& path);
	};
}
