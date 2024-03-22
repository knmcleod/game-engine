#include "GE/GEpch.h"

#include "Font.h"

#undef INFINITE
#include <msdfgen.h>
#include <msdfgen-ext.h>
#include <msdf-atlas-gen/msdf-atlas-gen.h>

namespace GE
{
	struct MSDFData
	{
		std::vector<msdf_atlas::GlyphGeometry> Glyphs;
		msdf_atlas::FontGeometry FontGeometry;
	};

	struct AtlasConfiguration
	{
		uint32_t Width;
		uint32_t Height;
		float Scale;
	};

	template<typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> func>
	static Ref<Texture2D> CreateAndCacheAtlas(const std::string& fontName, const AtlasConfiguration& atlasConfig,
		std::vector <msdf_atlas::GlyphGeometry> glyphs, msdf_atlas::FontGeometry fontGeometry)
	{
		msdf_atlas::GeneratorAttributes attributes;
		attributes.config.overlapSupport = true;
		attributes.scanlinePass = true;

		msdf_atlas::ImmediateAtlasGenerator<S, N, func, msdf_atlas::BitmapAtlasStorage<T, N>> atlasGenerator(atlasConfig.Width, atlasConfig.Height);
		atlasGenerator.setAttributes(attributes);
		atlasGenerator.setThreadCount(8);
		atlasGenerator.generate(glyphs.data(), glyphs.size());

		msdfgen::BitmapConstRef<T, N> bitmap = atlasGenerator.atlasStorage();
		TextureConfiguration config;
		config.Height = bitmap.height;
		config.Width = bitmap.width;
		config.InternalFormat = ImageFormat::RBG8;
		config.Format = DataFormat::RGB;
		config.GenerateMips = false;

		Ref<Texture2D> texture = Texture2D::Create(config);
		texture->SetData((void*)bitmap.pixels, bitmap.height * bitmap.width * (config.InternalFormat == ImageFormat::RBG8 ? 3 : 4));
		return texture;
	}

	Font::Font(const std::filesystem::path& fontPath)
	{
		m_MSDFData = CreateScope<MSDFData>();

		msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
		if (!ft)
		{
			GE_CORE_ERROR("Failed to load Font Freetype Handle");
			return;
		}

		std::string fontString = fontPath.string();
		msdfgen::FontHandle* font = msdfgen::loadFont(ft, fontString.c_str());
		if (!font)
		{
			GE_CORE_ERROR("Failed to load Font Handle");
			return;
		}

		struct CharsetRange
		{
			uint32_t Begin, End;
		};

		static const CharsetRange charsetRanges[] =
		{
			{ 0x0020, 0x00FF } // Basic Latin + Latin Supplement
		};

		msdf_atlas::Charset charset;
		for (CharsetRange range : charsetRanges)
		{
			for (uint32_t c = range.Begin; c <= range.End; c++)
				charset.add(c);
		}

		m_MSDFData->FontGeometry = msdf_atlas::FontGeometry(&m_MSDFData->Glyphs);
		int loadedGlyph = m_MSDFData->FontGeometry.loadCharset(font, 1.0, charset);
		
		float scale = 50.0;
		msdf_atlas::TightAtlasPacker atlasPacker;
		atlasPacker.setPixelRange(2.0);
		atlasPacker.setMiterLimit(1.0);
		atlasPacker.setSpacing(0.0);
		atlasPacker.setScale(scale);
		int remaining = atlasPacker.pack(m_MSDFData->Glyphs.data(), m_MSDFData->Glyphs.size());
		
		int width, height;
		atlasPacker.getDimensions(width, height);
		scale = atlasPacker.getScale();
		AtlasConfiguration atlasConfig;
		atlasConfig.Width = width;
		atlasConfig.Height = height;
		atlasConfig.Scale = scale;

		m_Texture = CreateAndCacheAtlas<uint8_t, float, 3, msdf_atlas::msdfGenerator>("Texture", atlasConfig,
			m_MSDFData->Glyphs, m_MSDFData->FontGeometry);

		if(false)
		{
			msdfgen::Shape shape;
			if (msdfgen::loadGlyph(shape, font, 'A'))
			{
				shape.normalize();

				msdfgen::BitmapRef<float, 3> bitmap;
				msdfgen::generateMSDF_legacy(bitmap, shape,
					4.0, msdfgen::Vector2(1.0), msdfgen::Vector2(4.0));
				msdfgen::saveTiff(bitmap, "output.png");
			}
		}

		destroyFont(font);
		deinitializeFreetype(ft);
	}

	Font::~Font()
	{

	}
}
