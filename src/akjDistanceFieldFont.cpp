#include "akjDistanceFieldFont.hpp"
#include "akjBinarySearchSequence.hpp"

namespace akj
{

	cCreateBitmapFontTask::cCreateBitmapFontTask(FreeTypeFace& face,
		uint32_t texture_size, uint32_t pad_prop, uint32_t gen_shift) 
		: cTask("Font creation: " + face.FamilyName() + " " + face.StyleName())
		, mLibrary(new FreeTypeLibrary)
		, mFace()
		, mOriginalFace(face)
		, mNextBoxIndex(0)
		, mTexSize(texture_size)
		, mNextPhase(kInitialPack)
		, kGenerationShift(pad_prop == 0 ? 0 : gen_shift)
		, kGenerationMult(pad_prop == 0 ? 1 : (1 << gen_shift))
		, kPaddingProportion(pad_prop)
	{
		mFace = std::move(face.Duplicate(*mLibrary));

		const float inv_tex_size = 1.0f / texture_size;
		mFaceName = mFace->getPSName();

		mGlyphs.reserve(256);
		//lets imagine that there are 94 characters

		mFace->GetAllASCIIGlyphs(mGlyphs);
		uint32_t max_char_code = 0;
		uint32_t min_char_code = 0xFFFFFFFF;
		for (const auto& glyph : mGlyphs)
		{
			if (glyph.CharCode > max_char_code) max_char_code = glyph.CharCode;
			if (glyph.CharCode < min_char_code) min_char_code = glyph.CharCode;
		}
		mGlyphsByCharCode.resize(max_char_code + 1 - min_char_code);
		mBaseCharCode = min_char_code;
		SetStatusString();
	}

	void cCreateBitmapFontTask::InitialSizingAndAllocation()
	{
		CalcSize(mGlyphs, mRects, mPack, mTexSize);

		ivec2 largest_rect = mRects.front().size;
		for (const auto& rect : mRects)
		{
			if (largest_rect.x < rect.size.x) largest_rect.x = rect.size.x;
			if (largest_rect.y < rect.size.y) largest_rect.y = rect.size.y;
		}
		mEDTBitmap = cAlignedBitmap(mTempBuffer,
			(2 * mPadding + largest_rect.width)*kGenerationMult,
			(2 * mPadding + largest_rect.height)*kGenerationMult,
			1, BIT_DEPTH_32);

		mLargeRenderBitmap = cAlignedBitmap(mLargeRenderBuffer,
			(2 * mPadding + largest_rect.width)*kGenerationMult,
			(2 * mPadding + largest_rect.height)*kGenerationMult,
			1, BIT_DEPTH_8);


		mSmallRenderBitmap = cAlignedBitmap(mSmallRenderBuffer,
			2 * mPadding + largest_rect.width,
			2 * mPadding + largest_rect.height,
			4, BIT_DEPTH_8);

		if(kPaddingProportion == 0)
		{
			mSmallRenderBitmap = mLargeRenderBitmap;
		}

		mDistanceBitmap = cAlignedBitmap(mDistanceBuffer,
			(2 * mPadding + largest_rect.width)*kGenerationMult,
			(2 * mPadding + largest_rect.height)*kGenerationMult,
			2, BIT_DEPTH_32);



		mFieldScale = static_cast<float>(mPadding);


		mOutBitmap = cAlignedBitmap(mTextureStorage, mTexSize, mTexSize,
															kPaddingProportion==0?4:4, BIT_DEPTH_8);

		if(kPaddingProportion >0)
		{
			mEDT.reset(new EuclideanDistanceTransform(mLargeRenderBitmap));
		}

		for (auto& pix : mOutBitmap.Pixels<pix::RGBA8>())
		{
			pix.a() = 0;
			pix.r() = 255;
			pix.g() = 0;
			pix.b() = 0;
		}

		//set up as higher resolution for render
		mFace->setPixelSizes(kGenerationMult*mGlyphSize,
												kGenerationMult*mGlyphSize);

		
		
	}

	int cCreateBitmapFontTask::CalcSize(
			const std::vector<FreeTypeFace::cGlyphIndex> &glyphs, 
			std::vector<cGlyphRect> &glyph_rects,
			std::vector<BinPacker::PackResult>& pack,
			uint32_t texture_size)
	{
		std::vector<iRect> rects;
		std::vector<ivec2> sizes;
		sizes.reserve(glyphs.size());
		rects.reserve(glyphs.size());
		glyph_rects.reserve(glyphs.size());
		const float inv_mult = 1.0f / kGenerationMult;
		const uint32_t pack_size = texture_size;
		const int min_size = 4;
		BinPacker bins;

		cMaximizingSearchSeq size_seq(min_size, 16);

		Log::Debug("Distance Field Font: Glyph Size  = %d", mGlyphSize);
		raw_ostream& outs = Log::Debug() << "\n\tAttempting padding sizes ";
		bool packed_ok = false;
		do{
			outs << size_seq << (size_seq.CurrentIncrement() > 1 ? ", " : ". ");
			pack.clear();
			rects.clear();
			sizes.clear();
			glyph_rects.clear();

			mFace->setPixelSizes(size_seq*kGenerationMult,
				size_seq*kGenerationMult);
			mFace->GetGlyphSizes(glyphs, rects);

			//convert to glyphrects
			// try to keep origin a whole number
			auto g_it = glyphs.begin();
			for (const iRect& r : rects)
			{
				if (g_it->CharCode == 'T')
				{
					bool y = true;
				}
				glyph_rects.emplace_back(r);
				ivec2 tr(r.Right(), r.Bottom());
				ivec2 bl(r.left, r.top);
				tr = tr.RoundedUpToMultiple(kGenerationMult) >> kGenerationShift;
				bl = bl.RoundedDownToMultiple(kGenerationMult) >> kGenerationShift;
				glyph_rects.back().origin = -bl;
				glyph_rects.back().size = tr - bl;
				++g_it;
			}

			// 1/8th of an em, rounded up
			const int padding = kPaddingProportion == 0 ? 0 : 
				((size_seq + kPaddingProportion - 1) / kPaddingProportion);

			for (const auto& r : glyph_rects)
			{
				// extra pixel border around every rect (for bilinear filtering)
				sizes.emplace_back(r.size.x + 2 * padding+2, r.size.y + 2 * padding+2);
			}
			
			packed_ok = bins.PackSingle(sizes, pack, pack_size);

			size_seq.IncreaseIfOK(packed_ok);
			
		} while (!size_seq.IsDone() || !packed_ok);
		
		AKJ_ASSERT_AND_THROW(packed_ok);
		mGlyphSize = size_seq;
		mPadding = kPaddingProportion == 0 ? 0 : 
			((mGlyphSize + kPaddingProportion - 1) / kPaddingProportion);
		outs << "Using size = " << size_seq << "\n";
		outs.flush();
		return mGlyphSize;
	}

	cCreateBitmapFontTask::eTaskPhase cCreateBitmapFontTask
		::RenderStage(uint32_t next_box, eTaskPhase phase)
	{
		uint32_t box = next_box;
		const BinPacker::PackResult& pr = mPack.at(box);
		uint32_t glyph_index = mGlyphs.at(pr.mID).index;
		uchar char_code = mGlyphs.at(pr.mID).CharCode;
		auto cc_it = mGlyphsByCharCode.begin()
			+ (char_code - mBaseCharCode);
		const cGlyphRect& rect = mRects.at(pr.mID);
		const float inv_tex_size = 1.0f / static_cast<float>(mOutBitmap.W());
		
		int x = pr.left+1 - rect.origin.x + (mPadding);
		int y = pr.top+1 + rect.origin.y + (mPadding);
		const cCoord2 texcoord = inv_tex_size * cCoord2(static_cast<float>(x),
			static_cast<float>(y));
		const cCoord2 glyphsize = inv_tex_size * cCoord2(rect.size);
		const cCoord2 offset = cCoord2(rect.origin);


		switch (phase)
		{
		case akj::cCreateBitmapFontTask::kPhaseInitialRender:
			AKJ_ASSERT(cc_it->mIndex == -1);
			*cc_it = cScaledGlyph(glyph_index, char_code,texcoord, offset, glyphsize);
			mLargeRenderBitmap.Clear(255);
			mSmallRenderBitmap.Clear(255);

			//AKJ_ASSERT(x >= 0, y >= 0);
			mFace->RenderCharOutlineInto(cc_it->mCharCode, mLargeRenderBitmap,
			(mPadding + rect.origin.x)*kGenerationMult,
			mLargeRenderBitmap.H() - (rect.origin.y + mPadding)*kGenerationMult);
			AKJ_ASSERT(mLargeRenderBuffer.BoundsIntact());
			
		//	mLargeRenderBitmap.ExportPNG("ex\\large_render" + Twine(mNextChar) + ".png");

			return kPaddingProportion == 0 
						? kPhaseDownsampleAndBlit 
						: kPhaseFirstDistance;

		case akj::cCreateBitmapFontTask::kPhaseFirstDistance:
		{
			mEDTBitmap.Clear(0);
			int edt_padding = std::max(mSmallRenderBitmap.H(),mSmallRenderBitmap.W());
			edt_padding *= kGenerationMult;
			auto edt_result = mEDT->RunTransform(static_cast<float>(edt_padding), 127);
			auto distances = mEDT->GetDistances();

			//save the distance transform
			memcpy(mEDTBitmap.Data(), edt_result.Data(), edt_result.Size());
			ForEachPixel([edt_padding](pix::AG32f& fp, const pix::RG16s dp, const pix::A32f edt)
				{
					if(edt.a() > 0.0f){
						fp.a() = dp.r();
						fp.gray() = dp.g();
					}
					else{
						fp.a() = 0.0f;
						fp.gray() = 0.0f;
					}
				},
				mDistanceBitmap.Pixels<pix::AG32f>(), distances.Pixels<pix::RG16s>(),
				mEDTBitmap.Pixels<pix::A32f>());
			
			AKJ_ASSERT(mTempBuffer.BoundsIntact());
			return kPhaseSecondDistance;
		}
		case akj::cCreateBitmapFontTask::kPhaseSecondDistance:
		{
			int edt_padding = std::max(mSmallRenderBitmap.H(),mSmallRenderBitmap.W());
			edt_padding *= kGenerationMult;
			auto edt_result = 
				mEDT->RunInverseTransform(static_cast<float>(edt_padding), 127);
			auto distances = mEDT->GetDistances();
			cAlignedBuffer temp;
			cAlignedBitmap signs = mDistanceBitmap;
			signs.UseAsStorage(temp);
			ForEachPixel(
				[](pix::AG32f& fp, const pix::RG16s& dp, 
						const pix::A32f& edt, pix::A32f& d_out)
			{
				if(edt.a() > 0.0f)
				{
					fp.a() += dp.r();
					fp.gray() += dp.g();
				}
				d_out.a() = d_out.a() - edt.a();
			},
			mDistanceBitmap.Pixels<pix::AG32f>(),
			distances.Pixels<pix::RG16s>(),
			edt_result.Pixels<pix::A32f>(),
			mEDTBitmap.Pixels<pix::A32f>());

			ForEachPixel([](pix::AG32f& signs, pix::AG32f& dist)
			{
				signs.a() = dist.a();
				dist.a() = std::abs(dist.a());
				signs.gray() = dist.gray();
				dist.gray() = std::abs(dist.gray());
			},
				signs.Pixels<pix::AG32f>(),
				mDistanceBitmap.Pixels<pix::AG32f>()
			);

			cAlignedBuffer smaller_buf;
			cAlignedBuffer temp_edt_buf;
			cAlignedBuffer temp_sign_buf;
			auto smaller_d = MipMapper::GenerateMipLevel(mDistanceBitmap, smaller_buf, 
																									kGenerationShift);
			auto smaller_edt = MipMapper::GenerateMipLevel(mEDTBitmap, temp_edt_buf, 
																									kGenerationShift);
			auto smaller_sign = MipMapper::GenerateMipLevel(signs, temp_sign_buf, 
																									kGenerationShift);
			const float inv_pad = 0.25f/static_cast<float>(mPadding);
			//mSmallRenderBitmap.ExportPNG("smaller_out.png");
			ForEachPixel([inv_pad]
			(const pix::A32f& edt, const pix::AG32f& dp, 
						pix::RGBA8& out, const pix::AG32f& signs)
			{
				out.r() =
					static_cast<uint8_t>(
					255.9f*std::min(std::max((inv_pad*edt.a())*0.5f+0.5f,0.0f), 1.0f));
				float s = signs.a() > 0.0f ? 1.0f : -1.0f;
				out.g() = static_cast<uint8_t>(255.9f*std::min(std::max(
						s*dp.a()*inv_pad*0.5f+0.5f,
					0.0f), 1.0f));
				s = signs.gray() > 0.0f ? 1.0f : -1.0f;
				out.b() = static_cast<uint8_t>(255.9f*std::min(std::max(
						s*dp.gray()*inv_pad*0.5f+0.5f,
					0.0f), 1.0f));
				out.a() = out.r()<255 ? 255 : 0; 
			},
				smaller_edt.Pixels<pix::A32f>(),
				smaller_d.Pixels<pix::AG32f>(),
				mSmallRenderBitmap.Pixels<pix::RGBA8>(),
				smaller_sign.Pixels<pix::AG32f>()
			);
			//mSmallRenderBitmap.ExportPNG("smaller_out.png");
			
			return kPhaseDownsampleAndBlit;		
		}
		case akj::cCreateBitmapFontTask::kPhaseDownsampleAndBlit:
		{
			AKJ_ASSERT(mLargeRenderBuffer.BoundsIntact());
			AKJ_ASSERT(mSmallRenderBuffer.BoundsIntact());

			cBitmap<1> src = mSmallRenderBitmap.SubRect<1>(
				0,  mSmallRenderBitmap.H()-rect.size.y-2*mPadding,
				rect.size.x+2*mPadding, rect.size.y+2*mPadding );

			cBitmap<1> dst = mOutBitmap.SubRect<1>(
				pr.left+1, pr.top+1, rect.size.x+2*mPadding,
				rect.size.y+2*mPadding);


			if(src.Comp() == 4)
			{
				src.ForEachPixel<uint32_t>([](uint32_t* src, uint32_t* dst)
				{
					*dst = *src;
				}, dst);
			}
			else{
				src.ForEachPixel<uint8_t>([](uint8_t* src, uint8_t* dst)
				{
					*dst++ = *src;
					*dst++ = *src;
					*dst++ = *src;
					*dst = 255;
				}, dst);
			}

			cBitmap<1> dst_pad = mOutBitmap.SubRect<1>(
				pr.left, pr.top, rect.size.x + mPadding * 2 +2,
				rect.size.y + mPadding * 2 +2);
			
			cAlignedBuffer buf;
			cBitmapExpander<pix::RGBA8> expander(buf);

			pix::RGBA8* ptr = reinterpret_cast<pix::RGBA8*>(
					mOutBitmap.PixelData(pr.left, pr.top)
			);
			ptr->g() =0; ptr->b() =0; ptr->r() = 255; ptr->a() =255;
			ptr = reinterpret_cast<pix::RGBA8*>(
					mOutBitmap.PixelData(pr.left+rect.size.x+2*mPadding+1, pr.top)
			); ptr->g() =0; ptr->b() =0; ptr->r() = 255; ptr->a() =255;
			ptr = reinterpret_cast<pix::RGBA8*>(
					mOutBitmap.PixelData(pr.left+rect.size.x+2*mPadding+1,
																pr.top+rect.size.y+2*mPadding+1)
			); ptr->g() =0; ptr->b() =0; ptr->r() = 255; ptr->a() =255;
			ptr = reinterpret_cast<pix::RGBA8*>(
					mOutBitmap.PixelData(pr.left, pr.top+rect.size.y+2*mPadding+1)
			); ptr->g() =0; ptr->b() =0; ptr->r() = 255; ptr->a() =255;

			expander.Expanded(dst_pad,5).ExportPNG(Twine("ex\\Expanded_")
																						+Twine(mNextChar)+".png");

			AKJ_ASSERT(mTextureStorage.BoundsIntact());
			return kNumRenderPhases;
		}
		case akj::cCreateBitmapFontTask::kNumRenderPhases:
		default:
			break;
		}

		return kNumRenderPhases;

	}

	

} // namespace akj