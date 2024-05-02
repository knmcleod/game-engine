#pragma once

#include "GE/Asset/Assets/Textures/Texture.h"

//#undef INFINITE
#include <msdfgen/msdfgen.h>
#include <msdfgen-ext.h>
#include <msdf-atlas-gen/msdf-atlas-gen.h>

namespace GE
{
	class Font : public Asset
	{
		friend class AssetSerializer;
	public:
		struct AtlasConfig
		{
			Ref<Texture2D> Texture = nullptr;

			uint32_t Width = 1;
			uint32_t Height = 1;
			float Scale = 1.0;

			uint64_t Seed = 0;
			int32_t ThreadCount = 8;
			bool ExpensiveColoring = false;
		};

		struct MSDFData
		{
			std::vector<msdf_atlas::GlyphGeometry> Glyphs;
			msdf_atlas::FontGeometry FontGeometry;
		};

		virtual inline const Asset::Type GetType() const { return Asset::Type::Font; }

		Font();
		~Font() override {};

		Ref<MSDFData> GetMSDFData() const { return m_MSDFData; }
		const AtlasConfig& GetAtlasConfig() const { return m_AtlasConfig; }
		Ref<Texture2D> GetAtlasTexture() const { return m_AtlasConfig.Texture; }
	private:
		Ref<MSDFData> m_MSDFData = nullptr;
		AtlasConfig m_AtlasConfig;
	};
}

