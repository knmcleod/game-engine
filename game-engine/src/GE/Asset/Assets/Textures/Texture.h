#pragma once

#include "GE/Asset/Asset.h"

#include "GE/Core/Buffer.h"
#include "GE/Core/Core.h"

#include <string>

namespace GE
{
	enum class ImageFormat
	{
		None = 0,
		R8,
		RBG8, // 3 channels
		RBGA8, // 4 channels
		RGBA32F
	};

	enum class DataFormat
	{
		None = 0,
		RGB, // 3 channels
		RGBA, // 4 channels 
	};

	struct TextureConfiguration
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		ImageFormat InternalFormat = ImageFormat::RBGA8;
		DataFormat Format = DataFormat::RGBA;
		bool GenerateMips = true;
	};

	class Texture : public Asset
	{
	public:
		virtual bool operator==(const Texture& other) const = 0;

		virtual TextureConfiguration GetConfig() const = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetID() const = 0;

		virtual void SetData(Buffer data) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void Unbind() const = 0;
	};

	class Texture2D : public Texture
	{
		friend class AssetSerializer;
	public:
		static Ref<Texture2D> Create(const TextureConfiguration& textureConfig, Buffer data = Buffer());

		static inline const Asset::Type GetAssetType() { return Asset::Type::Texture2D; }

		// Asset override
		virtual ~Texture2D() override { }
		virtual inline const Asset::Type GetType() const override { return GetAssetType(); }
	};

}
