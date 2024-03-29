#pragma once
#include "GE/MSDF/MSDF.h"
#include "GE/Rendering/Textures/Texture.h"

namespace GE
{
	class Font
	{
	public:
		Font(const std::filesystem::path& fontPath);
		~Font();

		Ref<MSDFData> GetMSDFData() const { return m_MSDFData; }
		Ref<Texture2D> GetTexture() const { return m_Texture; }
	
		static Ref<Font> GetDefault();
	private:
		Ref<MSDFData> m_MSDFData = nullptr;
		Ref<Texture2D> m_Texture = nullptr;
	};
}

