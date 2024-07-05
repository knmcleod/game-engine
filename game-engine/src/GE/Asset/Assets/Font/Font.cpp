#include "GE/GEpch.h"

#include "Font.h"

namespace GE
{
	Font::Font() : Font(UUID())
	{
	}

	Font::Font(UUID handle) : Asset(handle)
	{
		p_Type = Asset::Type::Font;
		m_MSDFData = CreateRef<MSDFData>();
	}

	Font::~Font()
	{
		m_MSDFData->Glyphs.clear();
	}

	Ref<Asset> Font::GetCopy()
	{
		GE_CORE_WARN("Could not copy Font Asset. Returning nullptr.");
		return nullptr;
	}

	uint64_t Font::GetByteArray(void* buffer /*= nullptr*/, uint64_t bufferSize /*= 0*/)
	{
		/*
		* - Type
		* ~ Config
		* * ~ Texture
		* * * - Type
		* * * - Name
		* * * ~ Config
		* * * * - Width
		* * * * - Height
		* * * * - InternalFormat
		* * * * - Format
		* * * * - GenerateMips
		* * * ~ Buffer
		* * * * - Size
		* * * * - Data
		* * - Width : uint32_t
		* * - Height : uint32_t
		* * - Scale : float
		* * - Seed : uint64_t
		* * - ThreadCount : int32_t
		* * - ExpensiveColoring : bool
		* - MSDFData
		* * - Glyphs : std::vector<msdf_atlas::GlyphGeometry>
		* * - Geometry : msdf_atlas::FontGeometry
		*/
		return 0;
	}
}
