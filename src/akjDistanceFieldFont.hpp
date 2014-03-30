#pragma once
#include "BinPacker.hpp"
#include "akjDistanceTransform.hpp"
#include "akjFreeTypeFace.hpp"
#include "Bitmap.hpp"
#include "akjExceptional.hpp"
#include "akjMipMapper.hpp"
#include "akjBitmapOperations.hpp"
#include "akjTask.hpp"
#include "akjIRect.hpp"

namespace akj
{
	struct tDeSerializerTrait;
	struct cGlyphRect
	{
		cGlyphRect(const iRect& rect)
			: size(rect.width, rect.height)
			, origin(-rect.left, rect.Bottom())
		{}
		ivec2 size;
		ivec2 origin;
	};

	struct cScaledGlyph
		{
			cScaledGlyph()
				: mCharCode(-1),mIndex(-1), mLocation(0.0f, 0.0f)
				, mOffset(0.0f, 0.0f), mSize(0.0f, 0.0f){}
			cScaledGlyph(int i, int c,const cCoord2& location, 
								cCoord2 offset, const cCoord2& size)
				: mCharCode(c), mIndex(i), mLocation(location)
				, mOffset(offset), mSize(size)
				 
			{}

			bool IsValid() const {return mIndex >= 0;}
			
			template <typename tWriter>
			void Serialize(tWriter& writer) const
			{
				writer.Write(mCharCode);
				writer.Write(mIndex);
				writer.Write(mLocation.x);
				writer.Write(mLocation.y);
				writer.Write(mOffset.x);
				writer.Write(mOffset.y);
				writer.Write(mSize.x);
				writer.Write(mSize.y);
			}
			
			template <typename tReader>
			cScaledGlyph(tReader& reader, const tDeSerializerTrait& )
			{
				reader.Read(mCharCode);
				reader.Read(mIndex);
				reader.Read(mLocation.x);
				reader.Read(mLocation.y);
				reader.Read(mOffset.x);
				reader.Read(mOffset.y);
				reader.Read(mSize.x);
				reader.Read(mSize.y);
			}
			int mCharCode;
			int mIndex;
			cCoord2 mLocation;
			cCoord2 mOffset;
			cCoord2 mSize;
		};

	


	class cDistanceFieldFont
	{
		public:
		enum eVersion { kSerializedVersion = 5 };
		

	

		cDistanceFieldFont(FreeTypeFace& face,const cAlignedBitmap& texture,
											 uint32_t glyph_size, float field_scale,
											 cStringRef face_name, std::vector<cScaledGlyph>&& glyphs)
			: mFace(face)
			, mGlyphsByCharCode(glyphs)
			, mFaceName(face_name)
			, mFieldScale(field_scale)
			, mGlyphSize(glyph_size)
		{
			mTextureBitmaps.emplace_back(texture);
			mTextureBitmaps.back().UseAsStorage(mTextureStorage);

			//copy texture data
			texture.ForEachPixel<uint32_t>([](const uint32_t* src, uint32_t* dst){
				*dst = *src;
			}, mTextureBitmaps.back());
			
			mBaseCharCode = mGlyphsByCharCode.front().mCharCode;

			PopulateIndexList();
		};

		~cDistanceFieldFont(){};

		void ExportGlyphs(const Twine& path = "")
		{
			int counter = 0;
			for(const cAlignedBitmap& bitmap : mTextureBitmaps)
			{
				bitmap.ExportTGA(path + mFaceName + "_glyphs" 
													+ Twine(counter) + ".tga");
				counter++;
			}
		}

		const cScaledGlyph& GlyphByIndex(uint32_t index)
		{
			if(index < mBaseIndex || index >= mBaseIndex + mGlyphsByIndex.size())
			{
				cStringRef g_name = mFace.GlyphName(index);
				Log::Error("unable to load glyph #%d \"%s\"", index,
					g_name.empty() ? "Unknown" : g_name);
			}

			const cScaledGlyph& g = mGlyphsByIndex[index - mBaseIndex];
			AKJ_ASSERT(g.IsValid());
			return g;
		}

		cStringRef FontName() const { return mFaceName; } 

		float TextureSize() const
		{
			return static_cast<float>(mTextureBitmaps[0].H());
		}

		float ScaleForPixelSize(float pixel_size)
		{
			return pixel_size/mGlyphSize;
		}

		float OuterPixelRange() const
		{
			return mFieldScale;
		}

		float InnerPixelRange() const
		{
			return mFieldScale;
		}

		const std::vector<cGlyphPos>& 
			ShapeText(cStringRef text, std::vector<cGlyphPos>& glyphs)
		{
			return mFace.ShapeText(text, glyphs);
		}

		const std::vector<cGlyphPos>& 
			ShapeText(cStringRef text)
		{
			return mFace.ShapeText(text);
		}

		const cAlignedBitmap& TextureBitmap() const
		{
			return mTextureBitmaps.front();
		}

		const FreeTypeFace& Face() const {return mFace;}

		template <class tWriter>
		void Serialize(tWriter& writer) const
		{
			writer.Write(static_cast<unsigned char>('D'));
			writer.Write(static_cast<unsigned char>('F'));
			writer.Write(static_cast<unsigned char>('F'));
			writer.Write(static_cast<uint32_t>(kSerializedVersion));
			auto checksum = writer.StartCheckSum();
			writer.Write(mFaceName);
			writer.Write(mGlyphSize);
			writer.Write(mFieldScale);
			cAlignedBuffer png_buf;
			mTextureBitmaps.front().ExportToPNGMem(png_buf);
			writer.Write(png_buf);
			//writer.Write(mTextureStorage);
			//writer.Write(mTextureBitmaps);
			writer.Write(mGlyphsByCharCode);
			writer.Write(mBaseCharCode);
		}

		static std::unique_ptr<cDistanceFieldFont>
			FromData(cStringRef data, FreeTypeFace& face)
		{
				AlignedBuffer<16> temp_storage;
				auto reader = cSerialization::ReaderCompressed(data, temp_storage);
				return FromReader(face, reader);
		}

		template <class tReader>
		static std::unique_ptr<cDistanceFieldFont>
			FromReader(FreeTypeFace& face, tReader& r)
		{
				std::unique_ptr<cDistanceFieldFont> dff;
				dff.reset(new cDistanceFieldFont(face, r, tReader::tFunction()));
				return dff;
		}

		uint32_t FaceHash() const
		{
			return mFace.FaceHash();
		}

		bool FaceEqual(const cDistanceFieldFont* other) const
		{
			return mFace.FaceEqual(&other->mFace);
		}


	private:
		void PopulateIndexList()
		{
			int max_index = 0;
			int min_index = std::numeric_limits<int>::max();
			for(const auto& g: mGlyphsByCharCode)
			{
				if(g.mIndex < 0) continue;
				if(g.mIndex > max_index) max_index = g.mIndex;
				if(g.mIndex < min_index) min_index = g.mIndex;
			}
			mBaseIndex = min_index;
			mGlyphsByIndex.resize(max_index-min_index+1);
			for(const auto& g: mGlyphsByCharCode)
			{
				if(g.IsValid())
				{
					cScaledGlyph& new_glyph = mGlyphsByIndex.at(g.mIndex - min_index);
					new_glyph = g;
				}
			}
		}


		template <class tReader>
		cDistanceFieldFont(FreeTypeFace& face, tReader& reader, 
												const tDeSerializerTrait&)
			:mFace(face)
			,mFaceName()
			,mGlyphSize()
			,mFieldScale()
			,mTextureStorage()
			,mTextureBitmaps()
			,mGlyphsByCharCode()
			,mBaseCharCode()
		{
			uint32_t version;
			char header[4] = {};
			reader.Read(header[0]);
			reader.Read(header[1]);
			reader.Read(header[2]);
			cStringRef header_string(&header[0]);
			if(header_string != header)
			{
				AKJ_THROW((Twine("Deserialized an invalid DFF file with header = \"")
					+header+"\""));
			}
			reader.Read(version);
			if(version < 4)
			{
				AKJ_THROW((Twine("Deserialized a DFF file with incorrect version #. "
					"Got ")+Twine(version)+", expected " + Twine(kSerializedVersion)));
			}
			reader.CheckSum();
			reader.Read(mFaceName);
			reader.Read(mGlyphSize);
			reader.Read(mFieldScale);
			if(version > 4)
			{
				cAlignedBuffer png_buf;
				reader.Read(png_buf);
				cStringRef png_data(png_buf.data(), png_buf.size());
				auto bitmap = cBitmapReader::ReadPNGFromMem(png_data, mTextureStorage);
				mTextureBitmaps.push_back(bitmap);
			}
			else{
				reader.Read(mTextureStorage);
				reader.Read(mTextureBitmaps);
			}
			reader.Read(mGlyphsByCharCode);
			reader.Read(mBaseCharCode);
			unsigned char* bitmap_data = mTextureStorage.data();
			// bitmaps deserialize with their data pointers unset
			// so we point them back at the right place here
			for (cAlignedBitmap& bm : mTextureBitmaps)
			{
				bm.SetData(bitmap_data);
				bitmap_data += bm.Size();
			}
			PopulateIndexList();
		}

		// size in pixels
		FreeTypeFace& mFace;
		std::string mFaceName;
		uint32_t mGlyphSize;
		float mFieldScale;
		AlignedBuffer<16> mTextureStorage;
		std::vector<cAlignedBitmap> mTextureBitmaps;
		std::vector<cScaledGlyph> mGlyphsByCharCode;
		std::vector<cScaledGlyph> mGlyphsByIndex;
		uint32_t mBaseCharCode;
		uint32_t mBaseIndex;
	};


class cCreateBitmapFontTask : public cTask
{
	enum eTaskPhase
	{
		kPhaseInitialRender,
		kPhaseFirstDistance,
		kPhaseSecondDistance,
		kPhaseDownsampleAndBlit,
		kNumRenderPhases,
		kInitialPack,
		kCleanup,
		kNoMorePhases
	};
	public:
	cCreateBitmapFontTask(FreeTypeFace& face, uint32_t texture_size,
												uint32_t pad_prop = 16, uint32_t gen_shift = 3);

	~cCreateBitmapFontTask()
	{
		bool destroy = true;
		
	}

	std::unique_ptr<cDistanceFieldFont> GetCompletedFont()
	{
		mOriginalFace.setPixelSizes(mGlyphSize, mGlyphSize);
		std::unique_ptr<cDistanceFieldFont> ret(
			new cDistanceFieldFont( mOriginalFace, mOutBitmap, mGlyphSize,
			 mFieldScale, mFaceName, std::move(mGlyphsByCharCode)));
		return ret;
	}

	virtual void DoWork(uint32_t thread_index = 0)
	{
		if(mNextPhase == kCleanup)
		{
			mFace.reset();
			mLibrary.reset();
			mOutBitmap.ExportPNG(mFaceName+Twine(mTexSize) + ".png");
			mNextPhase = kNoMorePhases;
		}
		else if(mNextPhase == kInitialPack)
		{
			InitialSizingAndAllocation();
			mNextPhase = kPhaseInitialRender;
			mNextChar = mGlyphs.at( mPack.at(mNextBoxIndex).mID).CharCode;
		}
		else
		{
			mNextPhase = RenderStage(mNextBoxIndex, mNextPhase);
			if(mNextPhase == kNumRenderPhases)
			{
				mNextPhase = kPhaseInitialRender;
				++mNextBoxIndex;

				
				if(mNextBoxIndex == mGlyphs.size())
				{
					mNextPhase = kCleanup;
				}
				else
				{
					mNextChar = mGlyphs.at( mPack.at(mNextBoxIndex).mID).CharCode;
				}
			}
		}
		SetStatusString();
	}

	void SetStatusString()
	{
		if(mNextPhase == kCleanup)
		{
			mStatusString = "Cleanup";
		}
		else if(mNextPhase == kInitialPack)
		{
			mStatusString = "choosing best packing size";
		}
		else if(mNextPhase == kPhaseInitialRender)
		{
			mStatusString.clear();
			mStatusString.push_back('\'');
			mStatusString.push_back(static_cast<unsigned char>(mNextChar));
			mStatusString.push_back('\'');
		}
	}

	virtual bool IsDone() const 
	{
		return mNextPhase == kNoMorePhases;
	}

	eTaskPhase RenderStage(uint32_t glyph_index, eTaskPhase phase);

	virtual cStringRef StatusString() const
	{
		return mStatusString;
	}

	virtual float Progress() const
	{
		const uint32_t total_glyphs = static_cast<uint32_t>(mGlyphs.size());
		if(mNextPhase == kInitialPack) return 0.0f;

		const float work_units = static_cast<float>(
							 (mNextBoxIndex+1) * kNumRenderPhases + (mNextPhase-1));
		const float total_work 
			= static_cast<float>(kNumRenderPhases * (total_glyphs + 2));

		return std::min(work_units/(total_work+1.0f), 1.0f);
	}

	void InitialSizingAndAllocation();
	int CalcSize(const std::vector<FreeTypeFace::cGlyphIndex> &glyphs,
									std::vector<cGlyphRect> &glyph_rects,
									std::vector<BinPacker::PackResult>& pack, 
									uint32_t texture_size);
	
	private:
	cAlignedBuffer mSmallRenderBuffer;
	cAlignedBuffer mTextureStorage;
	cAlignedBuffer mLargeRenderBuffer;
	cAlignedBuffer mTempBuffer;
	cAlignedBuffer mDistanceBuffer;
	
	cAlignedBitmap mLargeRenderBitmap;
	cAlignedBitmap mLargeOutBitmap;
	cAlignedBitmap mSmallRenderBitmap;
	cAlignedBitmap mOutBitmap;
	cAlignedBitmap mDistanceBitmap;
	cAlignedBitmap mEDTBitmap;

	std::unique_ptr<EuclideanDistanceTransform> mEDT;

	std::vector<cGlyphRect> mRects;
	std::vector<FreeTypeFace::cGlyphIndex> mGlyphs;
	uint32_t mPadding;
	std::vector<BinPacker::PackResult> mPack;
	uint32_t mNextBoxIndex;
	uint32_t mNextChar;
	std::string mStatusString;
	eTaskPhase mNextPhase;

	std::vector<cScaledGlyph> mGlyphsByCharCode;

	// we need another copy of this for threading purposes
	std::unique_ptr<FreeTypeLibrary> mLibrary;
	std::unique_ptr<FreeTypeFace> mFace;


	FreeTypeFace& mOriginalFace;
	std::string mFaceName;
	uint32_t mGlyphSize;
	uint32_t mBaseCharCode;
	uint32_t mTexSize;
	float mFieldScale;
	const uint32_t kGenerationShift;
	const uint32_t kGenerationMult;
	const uint32_t kPaddingProportion;

}; 

}

namespace std 
{
	template<> struct hash<const akj::cDistanceFieldFont*>
	{
		size_t operator()(const akj::cDistanceFieldFont* dff)
		{
			return dff->FaceHash();
		}
	};
	template <> struct equal_to<const akj::cDistanceFieldFont*> 
	{
		bool operator() (
			const akj::cDistanceFieldFont* lhs,
			const akj::cDistanceFieldFont* rhs) const 
		{
			return lhs->FaceEqual(rhs);
		}
		typedef const akj::cDistanceFieldFont* first_argument_type;
		typedef const akj::cDistanceFieldFont* second_argument_type;
		typedef bool result_type;
	};

}








