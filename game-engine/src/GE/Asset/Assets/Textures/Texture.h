#pragma once

#include "GE/Asset/Assets/Asset.h"

#include "GE/Core/Core.h"
#include "GE/Core/Memory/Buffer.h"

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
		* - RendererID : internal ID of texture
		* - Width : uint32_t
		* - Height : uint32_t
		* - InternalFormat : ImageFormat, default RGBA8/4 channels
		* - Format : DataFormat, default RGBA/4 channels
		* - GenerateMips : bool
		*/
		struct Config
		{
			uint32_t RendererID = 0;
			uint32_t Width = 1;
			uint32_t Height = 1;
			ImageFormat InternalFormat = ImageFormat::RGBA8;
			DataFormat Format = DataFormat::RGBA;
			bool GenerateMips = true;

			Buffer TextureBuffer = 0;

			Config() = default;
			Config(uint32_t width, uint32_t height, uint32_t channels, bool generateMips) : Width(width), Height(height), GenerateMips(generateMips)
			{
				if (channels == 3)
				{
					InternalFormat = Texture::ImageFormat::RGB8;
					Format = Texture::DataFormat::RGB;
				}
				else if (channels == 4)
				{
					InternalFormat = Texture::ImageFormat::RGBA8;
					Format = Texture::DataFormat::RGBA;
				}
			}
		};

		virtual bool operator==(const Texture& other) const = 0;

		virtual const Config& GetConfig() const = 0;
		virtual const uint32_t& GetWidth() const = 0;
		virtual const uint32_t& GetHeight() const = 0;
		virtual const uint32_t& GetID() const = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
		virtual void Unbind() const = 0;
	protected:
		virtual void SetData(Buffer data) = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Ref<Texture2D> Create(UUID handle, const Config& textureConfig, Buffer data = Buffer());

		// Asset override
		virtual ~Texture2D() override { }
	};

}
