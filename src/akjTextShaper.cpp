#include "akjTextShaper.hpp"
#include "hb.h"
#include "akjmisc.hpp"
#include "hb-ft.h"
#include "akjFreeTypeFace.hpp"

#define INV64f 0.015625f
namespace akj
{
	cTextShaper::cTextShaper(FreeTypeFace& face) :mFont(hb_ft_font_create(face.mFace, NULL))
		, mBuffer(hb_buffer_create())
		, mLineHeight(face.LineHeight())
	{
		AKJ_ASSERT_AND_THROW(mBuffer);
		mGlyphs.reserve(1024);
	}

	cTextShaper::~cTextShaper()
	{
		hb_buffer_destroy(mBuffer);
		hb_font_destroy(mFont);
	}

	std::vector<cGlyphPos>& cTextShaper
		::ShapeText(cStringRef text, std::vector<cGlyphPos>& glyphs)
	{
		hb_buffer_set_script(mBuffer, HB_SCRIPT_LATIN);
		hb_buffer_set_direction(mBuffer, HB_DIRECTION_LTR);
		float height = 0.0f;
		glyphs.reserve(text.size());
		glyphs.clear();
		if(text.empty()) return glyphs;
		do{
			auto pair = text.split('\n');
			text = pair.first;
			hb_buffer_add_utf8(mBuffer, text.data(), (long)text.size(),
												0, (long)text.size());
			hb_shape(mFont, mBuffer, NULL, 0);
			uint32_t glyph_count;
			hb_glyph_info_t *glyph_info
				= hb_buffer_get_glyph_infos(mBuffer, &glyph_count);
			hb_glyph_position_t *glyph_pos
				= hb_buffer_get_glyph_positions(mBuffer, &glyph_count);
			
			int x = 0;
			int y = 0;

			for (uint32_t i = 0; i < glyph_count; ++i)
			{
				glyphs.emplace_back(glyph_info[i].codepoint,
					(x+glyph_pos[i].x_offset)*INV64f,
					(y+glyph_pos[i].y_offset)*INV64f-height);
				if(glyph_pos[i].x_offset != 0)
				{
					bool crap = true;
				}
				x += glyph_pos[i].x_advance;
				y += glyph_pos[i].y_advance;
			}
			hb_buffer_reset(mBuffer);
			height += mLineHeight;
			text = pair.second;
		}while(!text.empty());
		return glyphs;
	}


	const std::vector<cGlyphPos>& cTextShaper::ShapeText(cStringRef text)
	{
		return ShapeText(text, mGlyphs);
	}


} // namespace akj
#undef INV64f