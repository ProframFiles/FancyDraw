#pragma once
#include <stdint.h>

namespace akj
{
	namespace pix
	{
		enum eFormat
		{
			kA8,
			kRG8,
			kRGB8,
			kRGBA8,
			kA16,
			kRG16,
			kRGBA16,
			kA32f,
			kAG32f,
			kRGBA32f
		};

		union A8
		{
			enum {kSize = 1, kComp = 1};
			uint8_t& a(){ return mVal; }
			uint8_t mVal;
			uint8_t mBytes[kSize];
		};
		AKJ_SIZE_CHECK(A8, A8::kSize);


		union RG8
		{
			enum {kSize = 2, kComp = 2};
			uint16_t mUint;
			uint8_t mBytes[kSize];
			uint8_t& r(){return mBytes[0];}
			uint8_t& g(){return mBytes[1];}
		};
		AKJ_SIZE_CHECK(RG8, RG8::kSize);

		union RGB8
		{
			enum {kSize = 3, kComp = 3};
			uint8_t mBytes[kSize];
			uint8_t& r(){return mBytes[0];}
			uint8_t& g(){return mBytes[1];}
			uint8_t& b(){return mBytes[2];}
		};
		AKJ_SIZE_CHECK(RGB8, RGB8::kSize);

		union RGBA8
		{
			enum {kSize = 4, kComp = 4};
			uint32_t mUint;
			uint8_t mBytes[kSize];
			uint8_t r() const {return mBytes[0];}
			uint8_t g() const {return mBytes[1];} 
			uint8_t b() const {return mBytes[2];} 
			uint8_t a() const {return mBytes[3];}
			uint8_t& r(){return mBytes[0];}
			uint8_t& g(){return mBytes[1];} 
			uint8_t& b(){return mBytes[2];} 
			uint8_t& a(){return mBytes[3];}
		};
		AKJ_SIZE_CHECK(RGBA8, RGBA8::kSize);

		union A16
		{
			enum {kSize = 2, kComp = 1};
			uint16_t& a(){ return mVal; }
			uint16_t mVal;
			uint8_t mBytes[kSize];
		};
		AKJ_SIZE_CHECK(A16, A16::kSize);

		union RG16
		{
			enum {kSize = 4, kComp = 2};
			uint16_t& r(){ return mVals[0]; }
			uint16_t& g(){ return mVals[1]; }
			uint16_t mVals[2];
			uint32_t mUInt;
			uint8_t mBytes[kSize];
		};
		AKJ_SIZE_CHECK(RG16, RG16::kSize);

		union RG16s
		{
			enum {kSize = 4, kComp = 2};
			int16_t& r(){ return mVals[0]; }
			int16_t& g(){ return mVals[1]; }

			const int16_t& r() const { return mVals[0]; }
			const int16_t& g() const { return mVals[1]; }
			int16_t mVals[2];
			uint32_t mUInt;
			uint8_t mBytes[kSize];
		};
		AKJ_SIZE_CHECK(RG16s, RG16s::kSize);

		union RGBA16
		{
			enum {kSize = 8, kComp = 4};
			uint64_t mUint;
			uint16_t mVals[4];
			uint8_t mBytes[kSize];
			uint16_t& r(){return mVals[0];}
			uint16_t& g(){return mVals[1];} 
			uint16_t& b(){return mVals[2];} 
			uint16_t& a(){return mVals[3];}
		};
		AKJ_SIZE_CHECK(RGBA16, RGBA16::kSize);

		union A32f
		{
			enum {kSize = 4, kComp = 1};
			float& a(){ return mFloat;}
			const float& a() const { return mFloat;}
			float mFloat;
			uint8_t mBytes[kSize];
		};
		AKJ_SIZE_CHECK(A32f, A32f::kSize);


		union AG32f
		{
			enum {kSize = 8, kComp = 2};
			float& a(){ return mVals[0];}
			float& gray(){ return mVals[1];}
			const	float& a() const { return mVals[0];}
			const float& gray() const { return mVals[1];}
			float mVals[2];
			uint64_t mUInt;
			uint8_t mBytes[kSize];
		};
		AKJ_SIZE_CHECK(AG32f, AG32f::kSize);

		union RGBA32f
		{
			enum {kSize = 16, kComp = 4};
			float& r(){return mVals[0];}
			float& g(){return mVals[1];} 
			float& b(){return mVals[2];} 
			float& a(){return mVals[3];}
			float mVals[4];
			uint64_t mUInts[2];
			uint8_t mBytes[kSize];
		};
		AKJ_SIZE_CHECK(RGBA32f, RGBA32f::kSize);
		
		struct tSizeFetcher
		{
			template <typename tPixel>
			uint32_t run() const {return tPixel::kSize;}
		};
		struct tCompFetcher
		{
			template <typename tPixel>
			uint32_t run() const {return tPixel::kComp;}
		};

		template <typename tFunctor> 
		inline uint32_t ApplyType(const tFunctor& func, eFormat format)
		{
			switch (format)
			{
			case akj::pix::kA8:
				return func.run<A8>();
				break;
			case akj::pix::kRGBA8:
				return func.run<RGBA8>();
				break;
			case akj::pix::kA16:
				return func.run<A16>();
				break;
			case akj::pix::kRG16:
				return func.run<RG16>();
				break;
			case akj::pix::kRGBA16:
				return func.run<RGBA16>();
				break;
			case akj::pix::kA32f:
				return func.run<A32f>();
				break;
			case akj::pix::kAG32f:
				return func.run<AG32f>();
				break;
			case akj::pix::kRGBA32f:
				return func.run<RGBA32f>();
				break;
			default:
				break;
			}
		}

		inline uint32_t SizeFromFormat(eFormat fmt)
		{
			return ApplyType(tSizeFetcher(), fmt);
		}

		inline uint32_t CompFromFormat(eFormat fmt)
		{
			return ApplyType(tCompFetcher(), fmt);
		}

		template <typename tColor, typename tPixel>
		inline void CopyColor(tPixel& dst, const tColor& src)
		{
			dst.r() = src.r;
			dst.g() = src.g;
			dst.b() = src.b;
			dst.a() = src.a;
		}

	}
}
