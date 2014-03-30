#pragma once
#include <stdint.h>
#include "Twine.hpp"
#include "akjAlignedBuffer.hpp"
#include "akjExceptional.hpp"
#include "akjPixelFormats.hpp"
#include "akjTypeTraits.hpp"

namespace akj{
	struct tDeSerializerTrait;

	enum eBitsPerComponent{
		BIT_DEPTH_NULL = 0,
		BIT_DEPTH_8 = 1,
		BIT_DEPTH_16 = 2,
		BIT_DEPTH_32 = 4,
		BIT_DEPTH_INVALID = 0xFFFFFFF0
	};

	struct AllowedConversionTag{};
	struct InvalidConversionTag{};
	struct InvalidCastTag{};
	struct AllowedCastTag{};

	// disallow general conversions
	template <uint32_t kOldAlignment, uint32_t kNewAlignment>
	struct tAlignmentChange{
		typedef InvalidConversionTag tConvertPermissionTag;
	};
	
	// allow conversions to the same alignment
	template <uint32_t kOldAlignment>
	struct tAlignmentChange<kOldAlignment, kOldAlignment>{
		typedef AllowedConversionTag tConvertPermissionTag;
	};

	// allow conversions from anything to Unaligned
	template <uint32_t kOldAlignment>
	struct tAlignmentChange<kOldAlignment, 1>{
		typedef AllowedConversionTag tConvertPermissionTag;
	};

	// allow conversions from 16 to 4
	template <> struct tAlignmentChange<16, 4>{
		typedef AllowedConversionTag tConvertPermissionTag;
	};

	// allow conversions from 64 to 16 and 4
	template <> struct tAlignmentChange<64, 16>{
		typedef AllowedConversionTag tConvertPermissionTag;
	};
	template <> struct tAlignmentChange<64, 4>{
		typedef AllowedConversionTag tConvertPermissionTag;
	};

	template <typename tCast, uint32_t kalign>
	struct tCast{ typedef InvalidCastTag tCastPermissionTag;};

	struct cBitmapRGB
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};
	// can we cast pixels of this bitmap to pixels of the specified type
	template <typename T> struct tCanCastPixelTrait{};

	template <uint32_t kAlign> struct tBitmapTraits
	{
		// no default traits
	};


	template <> struct tCast<uint32_t, 4>{
		typedef AllowedCastTag tCastPermissionTag;
	};
	template <> struct tCast<uint32_t, 16>{
		typedef AllowedCastTag tCastPermissionTag;
	};
	template <> struct tCast<float, 4>{
		typedef AllowedCastTag tCastPermissionTag;
	};
	template <> struct tCast<float, 16>{
		typedef AllowedCastTag tCastPermissionTag;
	};

//************************************
// Class:    Bitmap
// Description: Encapsulates a buffer holding image data 
// (does not own any data: the destructor does nothing)
// Template Argument: uint32_t kStrideGranularity
//		Each scanline of this bitmap is guaranteed to begin on a multiple of
//		kStrideGranularity
//************************************
template<uint32_t kRowAlignment>
class cBitmap
{
public:

	template <typename tPixel>
	struct iterator
	{
		iterator& operator ++()
		{
			++mCurrent;
			if(mCurrent == mRowEnd)
			{
				mRowEnd = reinterpret_cast<tPixel*>( 
					reinterpret_cast<uint8_t*>(mRowEnd)+mParent->Stride());
				mCurrent = mRowEnd - mParent->W();
			}
			return *this;
		}

		tPixel& operator*() {return *mCurrent;}
		tPixel* operator->() {return mCurrent;}

		bool operator ==(const iterator& other)
		{
			return mCurrent == other.mCurrent;
		}

		bool operator !=(const iterator& other)
		{
			return mCurrent != other.mCurrent;
		}

	private:
		iterator(const cBitmap<kRowAlignment> & parent)
			: mParent(&parent)
			, mCurrent(parent.Data().As<tPixel>())
			, mRowEnd(mCurrent+parent.W())
		{}
		iterator(const cBitmap<kRowAlignment> & parent, uint32_t x, uint32_t y)
			: mParent(&parent)
			, mCurrent(reinterpret_cast<tPixel*>(parent.PixelData(x, y)))
		{
			mRowEnd = mCurrent + mParent->W() - x+1;
		}
		friend class cBitmap<kRowAlignment>;
		tPixel* mCurrent;
		tPixel* mRowEnd;
		const  cBitmap<kRowAlignment>* mParent;
	};

		template <typename tPixel>
	struct upside_down_iterator
	{
		upside_down_iterator& operator ++()
		{
			++mCurrent;
			if(mCurrent == mRowEnd)
			{
				mRowEnd = reinterpret_cast<tPixel*>( 
					reinterpret_cast<uint8_t*>(mRowEnd) - mParent->Stride());
				mCurrent = mRowEnd - mParent->W();
			}
			return *this;
		}

		tPixel& operator*() {return *mCurrent;}
		tPixel* operator->() {return mCurrent;}

		bool operator ==(const upside_down_iterator& other)
		{
			return mCurrent == other.mCurrent;
		}

		bool operator !=(const upside_down_iterator& other)
		{
			return mCurrent != other.mCurrent;
		}

	private:
		upside_down_iterator(const cBitmap<kRowAlignment> & parent)
			: mParent(&parent)
			, mCurrent(reinterpret_cast<tPixel*>(parent.RowData(parent.H()-1)))
			, mRowEnd(mCurrent+parent.W())
		{}
		upside_down_iterator(const cBitmap<kRowAlignment> & parent, 
													uint32_t x, uint32_t y)
			: mParent(&parent)
			, mCurrent(reinterpret_cast<tPixel*>(parent.PixelData(x, y)))
		{
			mRowEnd = mCurrent + mParent->W() - x+1;
		}
		friend class cBitmap<kRowAlignment>;
		tPixel* mCurrent;
		tPixel* mRowEnd;
		const  cBitmap<kRowAlignment>* mParent;
	};

	template <typename tPixel>
	struct cPixels
	{
		typedef tPixel tPix;
		cPixels(const cBitmap& parent)
			: mParent(parent)
			, mEnd(parent, 0, parent.H())
		{
		}

		iterator<tPixel> begin() const
		{
			return iterator<tPixel>(mParent);
		}

		const iterator<tPixel>& end() const
		{
			return mEnd;
		}

		const iterator<tPixel> mEnd;
		const cBitmap<kRowAlignment>& mParent;
	};

	template <typename tPixel>
	struct cUpsideDownPixels
	{
		typedef tPixel tPix;
		cUpsideDownPixels(const cBitmap& parent)
			: mParent(parent)
			, mEnd(++upside_down_iterator<tPixel>(parent, parent.W()-1, 0))
		{
		}

		upside_down_iterator<tPixel> begin() const
		{
			return upside_down_iterator<tPixel>(mParent);
		}

		const upside_down_iterator<tPixel>& end() const
		{
			return mEnd;
		}

		const upside_down_iterator<tPixel> mEnd;
		const cBitmap<kRowAlignment>& mParent;
	};

	enum {
		kStrideRounding = kRowAlignment,
		kCharBits = 8
	};
	// these are the only valid choices
		
	cBitmap()
		:mHeight(0)
		,mWidth(0)
		,mNumComponents(0)
		,mStride(0)
		,mBPC(BIT_DEPTH_NULL)
		,mData(NULL)
	{
		InitChecks();
	}

	cBitmap( uint32_t w, uint32_t h, uint32_t comp, 
						uint32_t bpc, size_t stride = 0)
		:mHeight(h)
		,mWidth(w)
		,mNumComponents(comp)
		,mBPC(ConvertBitsToEnum(bpc))
		,mStride(StrideAlignmentRounded(stride==0 ? BytesPerLine() : stride))
		,mData(NULL)
	{
		InitChecks();
	}

	cBitmap(cAlignedBuffer& buf, uint32_t w, uint32_t h, uint32_t comp,
					uint32_t bpc, size_t stride = 0)
		:	mHeight(h)
		, mWidth(w)
		, mNumComponents(comp)
		, mBPC(ConvertBitsToEnum(bpc))
		, mStride(StrideAlignmentRounded(stride == 0 ? BytesPerLine() : stride))
		, mData(NULL)
	{
		InitChecks();
		UseAsStorage(buf);
	}

	// constructor with explicit stride. Stride will still be rounded up to a 
	// multiple of kRowAlignment
	cBitmap(uint8_t* data, uint32_t w, uint32_t h, 
					uint32_t comp, uint32_t bpc, size_t stride = 0)
		:mHeight(h)
		,mWidth(w)
		,mNumComponents(comp)
		,mBPC(ConvertBitsToEnum(bpc))
		,mStride(StrideAlignmentRounded(stride==0 ? BytesPerLine() : stride))
		,mData(data)
	{ 
		InitChecks();
	}

	cBitmap<kRowAlignment>& operator=(const cBitmap<kRowAlignment>& other)
	{
		mHeight = other.mHeight;
		mWidth = other.mWidth;
		mNumComponents = other.mNumComponents;
		mBPC = other.mBPC;
		mData = other.mData;
		mStride = other.mStride;
		return *this;
	}

	// ************************************
	// Special assignment operator for assignments that are from a different
	// stride granularity
	// this will not compile unless the assignment is actually allowed
	// Bitmap<4> = Bitmap<16> : allowed, because all 16 byte aligned buffers
	// are also 4 byte aligned
	// Bitmap<16> = Bitmap<4> : not allowed, and will not compile
	// ************************************
	template<uint32_t kOtherGranularity>
	cBitmap<kRowAlignment>& operator=(const cBitmap<kOtherGranularity>& other)
	{
		return (*this = other.template DowngradeAlignment<kRowAlignment>());
	}

	cBitmap(const cBitmap& other)
		:mHeight(other.mHeight)
		,mWidth(other.mWidth)
		,mNumComponents(other.mNumComponents)
		,mBPC(other.mBPC)
		,mData(other.mData)
		,mStride(other.mStride)
	{};

	// ************************************
	// constructor from something with stricter alignment constraint
	// ************************************
	template<uint32_t kOtherGranularity>
	cBitmap(const cBitmap<kOtherGranularity>& other)
	{ *this = other; };

	~cBitmap(){}

	cBitmap& SetData(uint8_t* data){
		mData = data;
		return *this;
	}

	bool IsValid() const
	{
		return (mBPC != BIT_DEPTH_NULL && mData != NULL);
	}

	void UseAsStorage(cAlignedBuffer & buffer)
	{
		buffer.reset(Size());
		SetData(buffer.data());
	}

	void Clear(uint8_t value) const
	{
		memset(Data(), value, Size()); // memset buffer to requested value.
	}

	cBitmap<kRowAlignment> GetSubSlice(uint32_t starting_row, 
																		uint32_t slice_height) const		
	{
		const size_t stride = Stride();
		assert(starting_row + slice_height <= mHeight);
		return cBitmap<kRowAlignment> (mData+starting_row*stride,
																		mWidth, slice_height,
																		mNumComponents, mBPC, mStride);
	}

	size_t Stride() const 
	{
		return mStride;
	}

	uint32_t BytesPerPixel() const 
	{
		return mBPC*mNumComponents;
	}

	uint32_t BPP() const
	{
		return mBPC*mNumComponents;
	}

	uint32_t BytesPerComponent() const 
	{ 
		return mBPC; 
	}
	uint32_t BPC() const
	{
		return mBPC;
	}

	uint32_t Height() const { 
		return mHeight;
	}

	uint32_t H() const {
		return mHeight;
	}

	uint32_t Width() const {
		return mWidth;
	}

	uint32_t W() const {
		return mWidth;
	}

	//************************************
	// Method:    NumComponents
	// Returns:   uint32_t
	//	 Returns the number of components for each pixel in the Bitmap
	//************************************
	uint32_t NumComponents() const {
		return mNumComponents; 
	}
	uint32_t Comp() const {
		return mNumComponents;
	}

	uint8_t* RowData(uint32_t line) const{
		return mData == NULL ? NULL : mData + line*mStride;
	}

	uint8_t* PixelData(int32_t x, int32_t y) const{
		return mData == NULL ? NULL : mData + y*mStride+mNumComponents*mBPC*x;
	}

	template <class tPixel>
	cPixels<tPixel> Pixels() const
	{
		return cPixels<tPixel>(*this);
	}
		
	template <class tPixel>
	cUpsideDownPixels<tPixel> PixelsUpsideDown() const
	{
		return cUpsideDownPixels<tPixel>(*this);
	}

	template<typename tPixel, class tFunctor>
	void ForEachPixel(tFunctor func) const
	{
		AKJ_ASSERT(sizeof(tPixel) == BPP());
		for (uint32_t y = 0; y < Height() ; ++y)
		{
			uint8_t * const start = mData + y * mStride;
			tPixel* const pix_row = reinterpret_cast<tPixel*>(start);
			tPixel* const end_ptr = pix_row + W();
			for (tPixel* row_ptr = pix_row; row_ptr < end_ptr; ++row_ptr)
			{
				func(*row_ptr);
			}
		}
	}




	template<typename tPixel, class tFunctor>
	void ForEachPixel(tFunctor func, cBitmap<kRowAlignment> other) const
	{
		assert(other.Width() == Width() && other.Height() == Height());
		const uint32_t pix_size = NumComponents()*BytesPerComponent();
		const uint32_t other_pix_size = other.NumComponents()
			*other.BytesPerComponent();

		for (uint32_t y = 0; y < Height(); ++y)
		{
			unsigned char* src = 
				reinterpret_cast<unsigned char*>(mData + y * mStride);
			unsigned char* dest = 
				reinterpret_cast<unsigned char*>(
				other.mData + y * other.mStride);
			unsigned char* const src_end = src +  BytesPerLine();
			for (; src < src_end; src += pix_size, dest += other_pix_size)
			{
				func(reinterpret_cast<tPixel*>(src),
							reinterpret_cast<tPixel*>(dest));
			}
		}
	}

	template<class tFunctor>
	void ForEachPixelFlipVert(tFunctor func, cBitmap<kRowAlignment> other) const
	{
		assert(other.Width() == Width() && other.Height() == Height());
		const uint32_t pix_size = NumComponents()*BytesPerComponent();
		const uint32_t other_pix_size = other.NumComponents()
			*other.BytesPerComponent();

		for (uint32_t y = 0; y < Height(); ++y)
		{
			unsigned char* src = 
				reinterpret_cast<unsigned char*>(mData + y * mStride);
			unsigned char* dest = 
				reinterpret_cast<unsigned char*>(
				other.mData + (other.Height()-y-1) * other.mStride);
			unsigned char* const src_end = src + mStride;
			for (; src < src_end; src += pix_size, dest += other_pix_size)
			{
				func(src, dest);
			}
		}
	}

	cDataPtr Data() const{
		return cDataPtr(mData); 
	}

	size_t Size() const{
		return Stride()*Height(); 
	}

	size_t SizeOfPixelData()
	{
		return BytesPerLine()*Height();
	}

	size_t BytesPerLine() const
	{
		return BytesPerLine(mWidth, mNumComponents, mBPC);
	}

	// returns true on success
	bool ExportTGA(const Twine& utf8_path) const;
	
	// returns true on success
	bool ExportPNG(const Twine& utf8_path) const;

	bool ExportJPEG(const Twine& utf8_path) const;

	bool ExportToJPEGMem(cAlignedBuffer& buf) const;
	bool ExportToPNGMem(cAlignedBuffer& buf) const;

	template <uint32_t kOutAlignment>
	cBitmap<kOutAlignment> SubRect(uint32_t x, uint32_t y, 
																uint32_t w, uint32_t h) const
	{
		AKJ_ASSERT(x+w <= W());
		AKJ_ASSERT(y+h <= H());
		uint8_t* data = mData;
		if(data)
		{
			data = PixelData(x, y);
		}
		return cBitmap<kOutAlignment>(
			data, w, h, mNumComponents, mBPC, mStride);
	}

	//************************************
	// Method:    DowngradeAlignment
	// Returns:   Bitmap<kOutStrideRounding>
	//		Returns a bitmap with the requested stride alignment (A different type
	//		than this bitmap)
	//		This is not possible in all cases, but factor of two downgrades are
	//		allowed
	//************************************
	template<uint32_t kOutStrideRounding>
	cBitmap<kOutStrideRounding> DowngradeAlignment() const
	{
		// we construct a tAlignmentChange object, which has a particular
		// typedef of the tConvertPermission tag
		// if the typedef resolves to AllowedConversionTag, then we have the
		// required function overload
		// otherwise, there is no appropriate downgrade implementation, and the
		// compile will fail
		return DowngradeAlignmentImpl<kOutStrideRounding>
			(typename tAlignmentChange<kRowAlignment,
			kOutStrideRounding>::tConvertPermissionTag());
	}

	void ConvertARGBToRGBA() const
	{
		AKJ_ASSERT(BytesPerComponent() == 1 && NumComponents() ==4);
		ForEachPixel([](unsigned char* pix){
			uint32_t new_val = pix[0] << 8;
			new_val |= pix[1] << 16;
			new_val |= pix[2] << 24;
			new_val |= pix[3];
			*reinterpret_cast<uint32_t*>(pix) = new_val;
		});
	}

	void ConvertBGRAToRGBA() const
	{
		AKJ_ASSERT(BytesPerComponent() == 1 && NumComponents() ==4);
		ForEachPixel([](unsigned char* pix){
			uint32_t new_val = pix[0] << 16;
			new_val |= pix[1] << 8;
			new_val |= pix[2];
			new_val |= pix[3] << 24;
			*reinterpret_cast<uint32_t*>(pix) = new_val;
		});
	}

	template <class tReader>
	cBitmap(tReader& r,const tDeSerializerTrait&)
	{
		r.Read(mHeight);
		r.Read(mWidth);
		r.Read(mNumComponents);
		uint32_t bpc = mBPC;
		r.Read(bpc);
		mBPC = static_cast<eBitsPerComponent>(bpc);
		r.ReadSize(mStride);
	}

	template <class tWriter>
	void Serialize(tWriter& w) const
	{
		w.Write(mHeight);
		w.Write(mWidth);
		w.Write(mNumComponents);
		w.Write(static_cast<uint32_t>(mBPC));
		w.WriteSize(mStride);
	}

private:
	// the compiler just has to run through here and not die
	// it doesn't need to be called or anything
	void CompileTimeChecks()
	{
		CompileErrorIfNotTrue<tNumberTraits<kRowAlignment>::tPowerOfTwo>();
	}
	void InitChecks() const
	{
		assert(mBPC != BIT_DEPTH_INVALID);
		assert(BytesPerLine() <= mStride);
		// does our stride have the required alignment?
		assert((mStride&(kStrideRounding-1)) == 0 );
		// does our data have the required alignment?
		assert(mData == 0 
			|| (reinterpret_cast<intptr_t>(mData)&(kStrideRounding-1)) == 0 );
	}


	template<uint32_t kOutStrideRounding>
	cBitmap<kOutStrideRounding> 
		DowngradeAlignmentImpl(AllowedConversionTag) const 
	{
		return cBitmap<kOutStrideRounding>(Data(),
																				Width(), Height(), 
																				NumComponents(), 
																				BytesPerComponent(), 
																				Stride() );
	}

	static size_t StrideAlignmentRounded(size_t stride){
		return (stride + kRowAlignment - 1) &~(kRowAlignment-1);
	}

	static size_t BytesPerLine(size_t width, uint32_t components, uint32_t bpc)
	{
		return (width*components*bpc);
	}

	// can't actually assign a bit depth of BIT_DEPTH_NULL
	// this is on purpose
	static eBitsPerComponent ConvertBitsToEnum(uint32_t bpc){
		switch (bpc)
		{
			case (BIT_DEPTH_8):
				return BIT_DEPTH_8;
			case (BIT_DEPTH_16):
				return BIT_DEPTH_16;
			case (BIT_DEPTH_32) :
				return BIT_DEPTH_32;
			case(BIT_DEPTH_NULL):
			default:
				AKJ_THROW("Attempted to assign an invalid value to "
					"an AKJ bitmap's bpc field");
			return BIT_DEPTH_INVALID;
					
		}
	}

	uint32_t mHeight;
	uint32_t mWidth;
	uint32_t mNumComponents;
	eBitsPerComponent mBPC;
	size_t mStride;
	uint8_t* mData;
};


	
typedef cBitmap<16> cAlignedBitmap;

class cBitmapWriter
{
public:
	static int WriteTGA(const cBitmap<1>& bitmap,const Twine& utf8_path);
	static int WritePNG(const cBitmap<1>& bitmap,const Twine& utf8_path);
	static int WriteJPEG(const cBitmap<1>& bitmap,const Twine& utf8_path);
	static int WriteJPEGToMem(const cBitmap<1>& bitmap, cAlignedBuffer& buf);
	static int WritePNGToMem(const cBitmap<1>& bitmap, cAlignedBuffer& buf);
};
class cBitmapReader
{
public:
	static cAlignedBitmap ReadPNGFromMem(cStringRef in, cAlignedBuffer& out);
	static cAlignedBitmap ReadBMPFromMem(cStringRef in, cAlignedBuffer& out);
};


template <uint32_t kRowAlignment>
inline bool cBitmap<kRowAlignment>
	::ExportTGA(const Twine& utf8_path) const
{
	return cBitmapWriter::WriteTGA(*this, utf8_path) != 0;
}

template <uint32_t kRowAlignment>
inline bool cBitmap<kRowAlignment>
	::ExportPNG(const Twine& utf8_path) const
{
	return cBitmapWriter::WritePNG(*this, utf8_path) != 0;
}

template <uint32_t kRowAlignment>
inline bool cBitmap<kRowAlignment>
	::ExportJPEG(const Twine& utf8_path) const
{
	return cBitmapWriter::WriteJPEG(*this, utf8_path) != 0;
}


template <uint32_t kRowAlignment>
inline bool cBitmap<kRowAlignment>
	::ExportToPNGMem(cAlignedBuffer& buf) const
{
	return cBitmapWriter::WritePNGToMem(*this, buf) >=0;
}

template <uint32_t kRowAlignment>
inline bool cBitmap<kRowAlignment>
	::ExportToJPEGMem(cAlignedBuffer& buf) const
{
	return cBitmapWriter::WriteJPEGToMem(*this, buf) >=0;
}

class cStoredBitmap
{
public:
	cStoredBitmap(const cAlignedBitmap& bitmap)
		:mBitmap(bitmap)
	{
		mBitmap.UseAsStorage(mStorage);
	}
	~cStoredBitmap(){}

	// we can't copy, but we can move
	cStoredBitmap& operator =(cStoredBitmap&& other)
	{
		mBitmap = other.mBitmap;
		mStorage = std::move(other.mStorage);
	}

	cAlignedBitmap& operator()()
	{
		return mBitmap;
	}

	cStoredBitmap(cStoredBitmap&& other)
		:mBitmap(other.mBitmap)
	{
		mStorage = std::move(other.mStorage);
	}

	operator cAlignedBitmap&()
	{
		return mBitmap;
	}

	cAlignedBuffer& Storage()
	{
		return mStorage;
	}

	static cStoredBitmap Copy(const cAlignedBitmap& other)
	{
		cStoredBitmap copy(other);
		AKJ_ASSERT_AND_THROW(other.Data());
		for(uint32_t row = 0; row < other.H(); ++row)
		{
			memcpy(copy().RowData(row), other.RowData(row), other.BytesPerLine());
		}
		return cStoredBitmap(std::move(copy));
	}

private:
	AKJ_NO_COPY(cStoredBitmap);
	cAlignedBitmap mBitmap;
	cAlignedBuffer mStorage;
};

} // namespace akj

