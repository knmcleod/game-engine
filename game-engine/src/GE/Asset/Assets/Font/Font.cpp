#include "GE/GEpch.h"

#include "Font.h"

namespace GE
{
	Font::Font(UUID handle, const AtlasConfig& config) : Asset(handle, Asset::Type::Font)
	{
		m_MSDFData = CreateRef<MSDFData>();
		m_AtlasConfig = config;
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
}
