#pragma once

#include "GE/Asset/Assets/Asset.h"

#include "GE/Core/Buffer.h"
#include "GE/Core/Core.h"

#include <string>

namespace GE
{
	class Texture : public Asset
	{
	public:
		enum class ImageFormat
		{
			None = 0,
			R8,
			RGB8, // 3 channels
			RGBA8, // 4 channels
			RGBA32F
		};

		enum class DataFormat
		{
			None = 0,
			RGB, // 3 channels
			RGBA, // 4 channels 
		};

		/*
		* Contains
		* - Name : std::string, name of file
		* - RendererID : ID of texture
		* - Width : uint32_t
		* - Height : uint32_t
		* - InternalFormat : ImageFormat default RGBA8/4 channels
		* - Format : DataFormat default RGBA/4 channels
		* - GenerateMips : bool
		*/
		struct Config
		{
			std::string Name = std::string("NewTexture");
			uint32_t Width = 1;
			uint32_t Height = 1;
			ImageFormat InternalFormat = ImageFormat::RGBA8;
			DataFormat Format = DataFormat::RGBA;
			bool GenerateMips = true;
		};

		virtual bool operator==(const Texture& other) const = 0;

		virtual Config GetConfig() const = 0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetID() const = 0;

		virtual const Buffer GetData() const = 0;
		virtual void SetData(Buffer data) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void Unbind() const = 0;
	};

	class Texture2D : public Texture
	{
		friend class AssetSerializer;
	public:
		static Ref<Texture2D> Create(const Config& textureConfig, Buffer data = Buffer());

		// Asset override
		virtual ~Texture2D() override { }
	};

}
