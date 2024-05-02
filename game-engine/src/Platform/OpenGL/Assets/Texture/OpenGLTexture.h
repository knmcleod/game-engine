#pragma once

#include "GE/Asset/Assets/Textures/Texture.h"

namespace GE
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		virtual bool operator==(const Texture& other) const override { return this->GetID() == ((Texture&)other).GetID(); };

		OpenGLTexture2D(const TextureConfiguration& textureConfig, Buffer data = Buffer());
		virtual ~OpenGLTexture2D() override;

		virtual TextureConfiguration GetConfig() const override { return m_Config; }
		virtual uint32_t GetWidth() const override { return m_Config.Width; }
		virtual uint32_t GetHeight() const override { return m_Config.Height; }
		virtual uint32_t GetID() const override { return m_RendererID; }

		virtual void SetData(Buffer data) override;

		virtual void Bind(uint32_t slot = 0) const override;
		virtual void Unbind() const override;

	private:
		uint32_t m_RendererID = 0;

		TextureConfiguration m_Config;
	};
}