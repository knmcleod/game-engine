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
	public:
		struct AtlasConfig
		{
			Ref<Texture2D> Texture = nullptr;

			float Scale = 1.0;

			uint64_t Seed = 0;
			uint32_t ThreadCount = 8;
			bool ExpensiveColoring = false;

			AtlasConfig() = default;
			AtlasConfig(const AtlasConfig& config) = default;
		};

		struct MSDFData
		{
			std::vector<msdf_atlas::GlyphGeometry> Glyphs = std::vector<msdf_atlas::GlyphGeometry>();
			msdf_atlas::FontGeometry FontGeometry;
		};
		
		Font(UUID handle = UUID(), const AtlasConfig& config = AtlasConfig(), Ref<MSDFData> data = nullptr);
		~Font() override;

		Ref<Asset> GetCopy() override;

		Ref<MSDFData> GetMSDFData() const { GE_CORE_ASSERT(m_MSDFData, "Font Data does not exist."); return m_MSDFData; }
		const AtlasConfig& GetAtlasConfig() const { return m_AtlasConfig; }
		Ref<Texture2D> GetAtlasTexture() const { return m_AtlasConfig.Texture; }
		const uint32_t& GetWidth() { return m_AtlasConfig.Texture->GetWidth(); }
		const uint32_t& GetHeight() { return m_AtlasConfig.Texture->GetHeight(); }

	private:
		Ref<MSDFData> m_MSDFData = nullptr;
		AtlasConfig m_AtlasConfig;
	};
}

