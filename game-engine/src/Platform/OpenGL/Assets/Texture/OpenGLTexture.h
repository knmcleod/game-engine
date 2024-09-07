#pragma once

#include "GE/Asset/Assets/Textures/Texture.h"

namespace GE
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		bool operator==(const Texture& other) const override { return this->GetID() == ((Texture&)other).GetID(); };

		OpenGLTexture2D(const Config& textureConfig, Buffer data = Buffer());
		~OpenGLTexture2D() override;

		// Asset overrides
		Ref<Asset> GetCopy() override;

		// Texture2D overrides
		const Config& GetConfig() const override { return m_Config; }
		const uint32_t& GetWidth() const override { return m_Config.Width; }
		const uint32_t& GetHeight() const override { return m_Config.Height; }
		const uint32_t& GetID() const override { return m_RendererID; }

		void Bind(uint32_t slot = 0) const override;
		void Unbind() const override;
	private:
		void SetData(Buffer data) override;

	private:
		uint32_t m_RendererID = 0;

		Config m_Config;
	};
}