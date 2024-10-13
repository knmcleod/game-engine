#include "GE/GEpch.h"

#include "Font.h"

namespace GE
{
	Font::Font(UUID handle, const AtlasConfig& config, Ref<MSDFData> data) : Asset(handle, Asset::Type::Font)
	{
		if (data)
			m_MSDFData = data;
		else
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
