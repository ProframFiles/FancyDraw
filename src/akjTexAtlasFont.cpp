#include "akjTexAtlasFont.hpp"
#include "BinPacker.hpp"
#include "akjBinarySearchSequence.hpp"
#include "akjSerialization.hpp"
#include "akjBitmapOperations.hpp"
#include <memory>

namespace akj
{

	void cTexAtlas::PopulateIndexList()
	{
		int max_index = 0;
		int min_index = std::numeric_limits<int>::max();
		for (const auto& g : mGlyphsByCharCode)
		{
			if (g.mIndex < 0) continue;
			if (g.mIndex > max_index) max_index = g.mIndex;
			if (g.mIndex < min_index) min_index = g.mIndex;
		}
		mBaseIndex = min_index;
		mGlyphsByIndex.resize(max_index - min_index + 1);
		for (auto& g : mGlyphsByCharCode)
		{
			if (g.IsValid())
			{
				auto& new_glyph = mGlyphsByIndex.at(g.mIndex - min_index);
				new_glyph = &g;
			}
		}
	}


	void cCreateTexAtlasTask
	::DeterminePacking(const std::vector<FreeTypeFace::cGlyphIndex>& glyphs)
	{
		std::vector<BinPacker::PackResult> pack;
		BinPacker bins;
		std::vector<iRect> rects;
		int max_x = 0;
		int max_y = 0;
		uint32_t root = 0;
		while(root*root <= glyphs.size()) { ++root;}
		mFace->GetGlyphSizes(glyphs, rects);
		std::vector<ivec2> sizes;
		sizes.reserve(rects.size());

		for(const auto& rect: rects)
		{
			sizes.emplace_back(rect.width+2, rect.height+2);
			if(rect.height > max_y) max_y = rect.height;
			if(rect.height > max_x) max_x = rect.width;
		}
		uint32_t max_dim = std::max(max_x, max_y);


		uint32_t best_size = cMinimizingSearchSeq::FindUpperEdge(
			0, root*(max_dim+2), [&](uint32_t trial_size)
		{
			pack.clear();
			return !bins.PackSingle(sizes, pack, trial_size);
		});

		pack.clear();
		const bool packed_ok = bins.PackSingle(sizes, pack, best_size);
		AKJ_ASSERT(packed_ok);


		mTexBitmap = cAlignedBitmap(mStorage, best_size, best_size, 1, BIT_DEPTH_8);

		//Tie all this information together
		// indices+charcodes from glyphs
		// sizes and offsets from rects
		// locations in the texture from my_pack()


		mGlyphsByCharCode.resize(glyphs.size());
		auto g_it = glyphs.begin();
		for(const auto& box: pack)
		{
			const auto& glyphIndex = glyphs.at(box.mID);
			const auto& rect = rects.at(box.mID);
			auto & final = mGlyphsByCharCode.at(box.mID);
			final.mCharCode = glyphIndex.CharCode;
			final.mIndex = glyphIndex.index;
			final.mTexRect = {box.left+1, box.top+1, rect.width, rect.height};
			final.mOffset = ivec2(-rect.left, rect.height+rect.top);
		}

	}

	void cCreateTexAtlasTask::RenderGlyphs()
	{
		cAlignedBuffer temp;
		cBitmapExpander<pix::A8> expander(temp);

		mTexBitmap.Clear(255);
		for(auto& glyph: mGlyphsByCharCode)
		{
			mFace->RenderCharOutlineInto(glyph.mCharCode, mTexBitmap,
			glyph.mTexRect.left+glyph.mOffset.x,
			glyph.mTexRect.top+glyph.mOffset.y-1);

			auto sub_bitmap = mTexBitmap.SubRect<1>(
				glyph.mTexRect.left-1, glyph.mTexRect.top-1,
				glyph.mTexRect.width+2, glyph.mTexRect.height+2);

			//make the offset from the bottom
			glyph.mOffset.y = glyph.mTexRect.height - glyph.mOffset.y;
			
			//expander.Expanded(sub_bitmap, 7)
			//	.ExportPNG("ex\\"+Twine(glyph.mCharCode)+"_"+Twine(mFontSize)+"px.png");
		}
		//mTexBitmap
		//	.ExportPNG(mFace->FamilyName() +"_All_"+Twine(mFontSize)+ "pix.png");
		//expander.Expanded(mTexBitmap, 5)
		//	.ExportPNG(mFace->FamilyName() +"_All_"+Twine(mFontSize)+ "pix_x5.png");
	}



	std::unique_ptr<cTexAtlas> cCreateTexAtlasTask::GetFinishedAtlas()
	{
		return std::unique_ptr<cTexAtlas> ( new cTexAtlas(
			std::move(mGlyphsByCharCode),
			mTexBitmap, std::move(mStorage),
			mFontSize
		));
		
	}


	void cTexAtlasFont::AddAtlas(std::unique_ptr<cTexAtlas>&& atlas)
	{
		if (!atlas)
		{
			Log::Warn("Unable to add void font to atlas font %s", mFaceName);
			return;
		}
		uint32_t new_size = atlas->mFontSize;
		cTexAtlas* new_atlas = GetAtlasForSize(new_size);
		if (!new_atlas)
		{
			Log::Debug("Adding new size %d for font %s", new_size, mFaceName);
			mAtlases[new_size] = std::move(atlas);
		}
		else
		{
			Log::Debug("Replacing existing size %d for font %s", new_size, mFaceName);
			mAtlases[new_size] = std::move(atlas);
		}
		mNeedTextureRepack = true;
	}

	void cTexAtlasFont::RepackAtlasTexture()
	{
		std::vector<ivec2> sizes;
		std::vector<cFixedSizeGlyph*> glyph_pointers;
		std::vector<cTexAtlas*> atlas_pointers;
		std::vector<BinPacker::PackResult> pack;
		
		// get the number of glyphs
		uint32_t total_rects = 0;
		for(auto& mapping: mAtlases)
		{
			const cTexAtlas& atlas = *mapping.second;
			total_rects += u32(atlas.mGlyphsByCharCode.size());
		}
		sizes.reserve(total_rects);
		glyph_pointers.reserve(total_rects);
		atlas_pointers.reserve(total_rects);
		pack.reserve(total_rects);

		// ok, ready to do some work
		uint32_t most_glyphs = 0;
		uint32_t largest_side = 0;		
		for(auto& mapping: mAtlases)
		{
			cTexAtlas& atlas = *mapping.second;
			
			most_glyphs += u32(atlas.mGlyphsByCharCode.size());
			
			for(auto& glyph: atlas.mGlyphsByCharCode)
			{
				sizes.emplace_back(glyph.mTexRect.size() + 2);
				glyph_pointers.emplace_back(&glyph);
				atlas_pointers.emplace_back(&atlas);
				if(sizes.back().x > (int)largest_side) largest_side = sizes.back().x;
				if(sizes.back().y > (int)largest_side) largest_side = sizes.back().y;
			}
		}

		uint32_t max_per_side= 0;
		while(max_per_side*max_per_side <= most_glyphs) { ++max_per_side;}
		const uint32_t max_tex_side = max_per_side*(largest_side);

		BinPacker bins;

		uint32_t best_size = cMinimizingSearchSeq::FindUpperEdge(
			0, max_tex_side, [&](uint32_t trial_size)
		{
			pack.clear();
			return !bins.PackSingle(sizes, pack, trial_size);
		});

		//round up to x16 so that our stride matches the width (for OGL)
		best_size = ((best_size+15)/16)*16;

		bins.PackSingle(sizes, pack, best_size);

		cAlignedBuffer new_tex_buf;
		cAlignedBitmap new_tex(new_tex_buf, best_size, best_size, 1, BIT_DEPTH_8);
		new_tex.Clear(255);

		for(const auto& box: pack)
		{
			cFixedSizeGlyph& glyph = *glyph_pointers.at(box.mID);
			cTexAtlas& atlas = *atlas_pointers.at(box.mID);
			atlas.BlitGlyph(glyph.mCharCode, new_tex, box.left+1, box.top+1);
			glyph.mTexRect.left = box.left+1;
			glyph.mTexRect.top = box.top+1;//+glyph.mTexRect.height;
			
		}
		for(auto& mapping: mAtlases)
		{
			cTexAtlas& atlas = *mapping.second;
			atlas.mTexture = new_tex;
			atlas.mStorage.Deallocate();
		}
		
		mTextureStorage = std::move(new_tex_buf);
		mTextureBitmap = new_tex;
		mNeedTextureRepack = false;
		mTextureBitmap.ExportPNG("TexAtlas.png");
		
	}

	bool cTexAtlasFont::SetPixelSize(uint32_t size)
	{
		if (mActiveAtlas && mActiveAtlas->mFontSize == size)
		{
			return true;
		}
		else if (cTexAtlas* atlas = GetAtlasForSize(size))
		{
			mActiveAtlas = atlas;
			mFace.setPixelSizes(mActiveAtlas->mFontSize, mActiveAtlas->mFontSize);
			return true;
		}
		return false;
	}

} // namespace akj