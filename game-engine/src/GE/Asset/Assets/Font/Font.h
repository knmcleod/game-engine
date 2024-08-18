#pragma once

#include "GE/Asset/Assets/Textures/Texture.h"

// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include <msdfgen/msdfgen.h>
#include <msdfgen-ext.h>
#include <msdf-atlas-gen/msdf-atlas-gen.h>
#pragma warning(pop)

namespace GE
{
	class Font : public Asset
	{
		friend class AssetSerializer;
	public:
		struct AtlasConfig
		{
			Ref<Texture2D> Texture = nullptr;

			// Remove. Use AssetMetadata.FilePath.Name instead
			std::string Name = std::string("NewFont");
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

		Font();
		Font(UUID handle);
		~Font() override;

		Ref<Asset> GetCopy() override;

		Ref<MSDFData> GetMSDFData() const { GE_CORE_ASSERT(m_MSDFData, "Font Data does not exist."); return m_MSDFData; }
		const AtlasConfig& GetAtlasConfig() const { return m_AtlasConfig; }
		Ref<Texture2D> GetAtlasTexture() const { return m_AtlasConfig.Texture; }
	private:
		Ref<MSDFData> m_MSDFData = nullptr;
		AtlasConfig m_AtlasConfig;
	};
}

